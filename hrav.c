#include <stdio.h>
#include <stdlib.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
void print_hex(unsigned char* buf, int count);


int main(int argc, char** argv)
{
	int sock, n;
	char buffer[2048];
	char sendbuf[2048];
	struct ifreq ethreq;
	struct sockaddr_ll saddr;
	int packet_num = 1;
        int i;

        for(i = 0; i < 2048; i++)
        {
            buffer[i] = 0;
            sendbuf[i] = 0;
        }
	if ( (sock=socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))<0){
		perror("socket");
		exit(1);
	}
	/* Set the network card in promiscuos mode */
	strncpy(ethreq.ifr_name,"nf0",IFNAMSIZ);
	if (ioctl(sock,SIOCGIFINDEX,&ethreq)==-1) {
		perror("ioctl");
		close(sock);
		exit(1);
	}
	
	saddr.sll_family = AF_PACKET;
    saddr.sll_protocol = htons(ETH_P_ALL);
    saddr.sll_ifindex = ethreq.ifr_ifindex;

    if (bind(sock, (struct sockaddr*)(&(saddr)), sizeof(saddr)) < 0)
    {
        perror("Bind");
		close(sock);
		exit(1);
    }
    	sendbuf[0] = 0xfe;sendbuf[1] = 0xca;sendbuf[2] = 0xae;
    	sendbuf[7] = 0x03;
    	sendbuf[32] = 0x0a;sendbuf[33] = 0x0b;sendbuf[34] = 0x0c;sendbuf[35] = 0x0d;
    	sendbuf[36] = 0x1a;sendbuf[37] = 0x1b;sendbuf[38] = 0x1c;sendbuf[39] = 0x1d;
//    	sendbuf[32] = 1;sendbuf[33] = 0;sendbuf[34] = 0xad;sendbuf[35] = 0xde;
    	
 	if(sendto(sock, sendbuf,100, 0, NULL, 0) <= 0) printf("Error send\n");;
	while (1) {
		n = recvfrom(sock,buffer,2048,0,NULL,NULL);
		printf("Packet%d -----------------------------------------------------\n", packet_num);
		print_hex(buffer, n);
		packet_num++;
	}
	return 0;
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


