#include "hrav_protocol.h"


int open_device(char* iface_name){
	//static char *iface_name = "nf0";
	struct ifreq ifr;
	struct sockaddr_ll saddr;
	int sockfd;
	int error;

    /* create socket */
	sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (sockfd == -1) {
		printf("Socket failed!\n");
		exit(1);
	}
	
	printf("Found iface %s\n", iface_name);
	
	bzero(&ifr, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);

	if (ioctl(sockfd, SIOCGIFINDEX, &(ifr)) < 0)
	{
		printf("Ioctl error!\n");
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
		error = 1;
		return -1;
	}
	
	bzero(&saddr, sizeof(struct sockaddr_ll));
	saddr.sll_family = AF_PACKET;
	saddr.sll_protocol = htons(ETH_P_ALL);
	saddr.sll_ifindex = ifr.ifr_ifindex;

	if (bind(sockfd, (struct sockaddr*)(&(saddr)), sizeof(saddr)) < 0)
	{
		printf("Bind error!");
		shutdown(sockfd, SHUT_RDWR);
		close(sockfd);
		error = 1;
		return -1;
	}
	return sockfd;
}

void PrintInHex(char *mesg, unsigned char *p, int len)
{
	printf("%s\n",mesg);
	while(len--)
	{
		fprintf(stdout, "%.2X\t", *p);
		p++;
	}
	printf("\n");
}

int send_data(int sockfd, const char* sendbuff, int len)
{
    /* Sending packet */
    int written_bytes = write(sockfd,sendbuff,len); // cli_bm_scanbuff se duoc thay the boi ham write nay !!!

	if (written_bytes <= 0)
	{
		printf("Write data error!\n");
		return 1;
	}
    else
    {
		//printf("Write bytes: %d\n",written_bytes);
	}

	return 0;
}

int hrav_send_buff(int sockfd, int bufferID, char* sendbuff, int send_buffer_length)
{

    PACKET sendPacket;
    BUFFER sendBuffer;
	
	//printf("Size of struct header %d packet %d buffer %d \n",sizeof(HEADER),sizeof(PACKET),sizeof(BUFFER));

	// Handle buffer segmentation.
	//char sendbuff[2048] = "\xCA\xAE\xFE\x01\xFF\xFF\xFF\x4\x1B\xED\xA5\x39\x65\xB3\x24\x39\x4A\xA3\xF1\x5A\x6E\xBF\xC1\xB4\xF4\xE5\x35\xDA\xCE\x4D\x38\xD2\xA6\x2C\x5E\xA3\xB4\x53\x15\x3F\x5B\x4A\x7F\x83\x11\x59\x49\xFF\x8E\xFB\xEC\xA9\x5A\x24\x2C\x58\x75\x75\x7A\xC6\x15\x66\x52\xD3\xC2\xB3\xA9\x3D\x11\xA8\x6B\x2A\x53\x2F\x61\x42\x9A\xC5\xEC\x9A\x72\x1C\xBE\xCB\xE7\x58\xEA\xA7\x43\x17\x66\x6E\x96\x1C\xF4\xD8\x21\xCF\x46\x8E\xBB\x2F\xE8\x39\xC5\x1F\x1F\x95\x31\x99\xE4\x6E\x3C\xB2\x8A\x91\x81\xF2\x8C\x8B\xEE\xE7\x86\xBA\xE1\xF9\x5E\xE4\xF4\x5F\xF1\xC9\x7C\x43\xB6\x32\xF7\xE9\x5D\xEC\x8C\x67\xDE\xBF\x16\xAF\xAB\xC2\x45\xD1\xD4\x6E\x6F\xEF\xFB\x7E\x7E\x3B\xAE";
	//int bufferID = 0;
	//int send_buffer_length= length;
	//printf("send_buffer_length = %d\n",send_buffer_length);
	int pos = 0;
	char first = TRUE;
	char last = FALSE;
	memset(sendPacket.info, '\xFF', DMA_BUFF_INFO);

//	bzero(sendPacket.info,DMA_BUFF_INFO);
	// Prepare packet
	while(!last)
	{
		//PrintInHex("sendbuf after while ", sendbuff, 4);
		/* Clear all sending buffer */
		sendPacket.length = 0;
//		PrintInHex("sendbuf after sendpacket.length", sendbuff, 4);
		bzero(sendPacket.data,DMA_BUF_SIZE);
//		PrintInHex("sendbuf after bzero 1", sendbuff, 4);
		sendBuffer.length = 0;
		bzero(sendBuffer.buffer,DMA_BUF_SIZE);
//		PrintInHex("sendbuf after bzero ", sendbuff, 4);
		//calculate data length
		int dataSize;
		if(first){
			dataSize = DMA_PKT_LEN - sizeof(HEADER) - sizeof(sendPacket.info);
		}else{
			dataSize = DMA_PKT_LEN - sizeof(HEADER);
		}
		last = (dataSize<(send_buffer_length - pos))?FALSE:TRUE;
		dataSize = last?(send_buffer_length-pos):dataSize;

		//calculate packet length
		int packetSize;
		packetSize = dataSize + sizeof(HEADER) +( first?sizeof(sendPacket.info):0);
		/* Configure packet Header */
		sendPacket.header.magic[0] = DMA_PKT_MAGIC_NUMBER_0;
		sendPacket.header.magic[1] = DMA_PKT_MAGIC_NUMBER_1;
		sendPacket.header.magic[2] = DMA_PKT_MAGIC_NUMBER_2;

		sendPacket.header.type = 0x01;
//		sendPacket.header.bufferID[0] = 0xFF & bufferID;
//		sendPacket.header.bufferID[1] = 0xFF & (bufferID>>8);
//		sendPacket.header.bufferID[2] = 0xFF & (bufferID>>16);
		sendPacket.header.bufferID[0] = 0xFF;
		sendPacket.header.bufferID[1] = 0xFF;
		sendPacket.header.bufferID[2] = 0xFF;
		// Configure packet status
		if(packetSize >= MIN_DMA_PKT_LEN){
			sendPacket.header.status &= 0x00; // set the 6bit MSB = 0.
		}else{
			sendPacket.header.status = 0xFC & (packetSize << 2); // set 6bit MSB = packetsize;
		}
		sendPacket.header.status = (first)?(sendPacket.header.status|0x02):(sendPacket.header.status & 0xFD); //if first set status[1] to 1.
		sendPacket.header.status = (last)?(sendPacket.header.status|0x01):(sendPacket.header.status & 0xFE); //if last set status[0] to 1.

		/* Configure Buffer Info */
		if(first){
			//setting extra data for current data bufer at the first packet
		}

		/* Configure packet data */
		memcpy(sendPacket.data,sendbuff + pos,dataSize);
		sendPacket.length = dataSize;
//		PrintInHex("sendbuf pos ", sendbuff, 4);
//		PrintInHex("sendpacket data ",sendPacket.data,4);

		/* Copy packet to sending buffer */
		//copy header
		memcpy(sendBuffer.buffer,&sendPacket.header,sizeof(HEADER));
		//copy buffer info and data to send
		if(first)
		{
			//copy buffer info
			memcpy(sendBuffer.buffer+sizeof(HEADER),sendPacket.info,DMA_BUFF_INFO);
			//copy buffer data.
			memcpy(sendBuffer.buffer+sizeof(HEADER)+DMA_BUFF_INFO,sendPacket.data,sendPacket.length);
		}
		else
		{
			memcpy(sendBuffer.buffer+sizeof(HEADER),sendPacket.data,sendPacket.length);
		}
		sendBuffer.length = packetSize;
//		printf("sendBuffer.buffer = %s\n",sendBuffer.buffer);
		//printf("sendBuffer.length = %d \n",sendBuffer.length);

		/* Begin transfer*/
    	if(send_data(sockfd, sendBuffer.buffer, sendBuffer.length)){
    		printf("hrav_send_buff sending error \n");
    		return 1;
    	}

		//PrintInHex("packet_buffer : ",sendBuffer.buffer,sendBuffer.length);

    	pos += dataSize;
		first=FALSE;
	}
	return 0;
}



int open_device_receive(char* iface_name){
	int sock, n;
	struct ifreq ethreq;
	struct sockaddr_ll saddr;
	int packet_num = 1;

	if ( (sock=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))<0){
		perror("recive socket");
		exit(1);
	}
	/* Set the network card in promiscuos mode */
	strncpy(ethreq.ifr_name,iface_name,IFNAMSIZ);
	if (ioctl(sock,SIOCGIFINDEX,&ethreq)==-1) {
		perror("recive ioctl");
		close(sock);
		exit(1);
	}
	
	saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_ifindex = ethreq.ifr_ifindex;

    if (bind(sock, (struct sockaddr*)(&(saddr)), sizeof(saddr)) < 0)
    {
        perror("recive Bind");
		close(sock);
		exit(1);
    }

	return sock;
}

void print_hex(unsigned char* buf, int count)
{
	int i;
	for(i = 0; i < count; i++)
	{
		printf("%x ", buf[i]);
		if((i&0x1F) == 31) printf("\n");
	}
	printf("\n");
}


int hrav_receive_buff(int sockfd, int* bufferID, char* receivebuff, int* buffer_length){
	int n;
	int packet_num=0;
	char have_packet = FALSE;
	while(1) {
		n = recvfrom(sockfd,receivebuff,2048,0,NULL,NULL);
		printf("Packet%d -----------------------------------------------------\n", packet_num);
		packet_num ++;
		print_hex(receivebuff, n);
		if(receivebuff[0]==0xfe && receivebuff[1] == 0xca && receivebuff[2] == 0xae){
			have_packet = TRUE;
			break;
		}
	}
	*bufferID = receivebuff[6]<<16 & receivebuff[5]<<8 & receivebuff[4];
	*buffer_length = n;
	if(have_packet){
		printf("FIND correct one with iD %d \n", *bufferID);
	}

	return 0;	
}
