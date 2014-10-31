#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "serial.h"
#include "stdio.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>


#undef PDEBUG
#ifdef PC_DEBUG
#define PDEBUG(fmt, args...) printf(fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif

#define DMA_PKT_LEN 		1472
#define DMA_PKT_START_LEN	8
#define DMA_PKT_HEADER		8
#define	DMA_PKT_MAGIC_NUMBER_0	0xfe
#define DMA_PKT_MAGIC_NUMBER_1	0xca
#define DMA_PKT_MAGIC_NUMBER_2	0xae
#define DMA_BUF_SIZE (DMA_PKT_LEN + 1)
#define DEFAULT_IFACE	"nf3"

static int is_send = 0;
static int is_recv = 0;

static char *snd_filename = NULL;
static char *rcv_filename = NULL;
static char *iface_name = DEFAULT_IFACE;

static unsigned long send_number = 0;
static unsigned long recv_number = 0;

int stop = 0;
int i, is_first;
long pktsize, filesize, datasize, position;
char packet[DMA_BUF_SIZE];

int s;
int error;

void send_data(int *value);
void receive_data(int *value);
void processArgs (int, char ** );
void usage ();

int main(int argc, char *argv[])
{
	
	struct ifreq ifr;
	struct sockaddr_ll saddr;
	
	FILE *fin  = NULL;
	FILE *fout = NULL;
	
		
    /* Argument processing */
	processArgs (argc, argv);
	
    /* create socket */
	s = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	if (s == -1) {
		printf("Socket failed!\n");
		exit(1);
	}
	
	printf("Found iface %s\n", iface_name);
	
	bzero(&ifr, sizeof(struct ifreq));
	strncpy(ifr.ifr_name, iface_name, IFNAMSIZ);

	if (ioctl(s, SIOCGIFINDEX, &(ifr)) < 0)
	{
		printf("Ioctl error!\n");
		shutdown(s, SHUT_RDWR);
		close(s);
		error = 1;
		return;
	}
	
	bzero(&saddr, sizeof(struct sockaddr_ll));
	saddr.sll_family = AF_PACKET;
	saddr.sll_protocol = htons(ETH_P_ALL);
	saddr.sll_ifindex = ifr.ifr_ifindex;

	if (bind(s, (struct sockaddr*)(&(saddr)), sizeof(saddr)) < 0)
	{
		printf("Bind error!");
		shutdown(s, SHUT_RDWR);
		close(s);
		error = 1;
		return;
	}
	
	pthread_t threads[2];
	int param[2];
	for (i = 0; i < 2; i++) 
		param[i] = i;
	//if (serial_Open("/dev/ttyUSB0") != 0) {
	 // printf("connect error!");
	//	return 0;
	//}
	// long q=5;
	// while (q>0){
	// 	writeData("r");
	// 	q--;
	// }
	// sleep(1);
	time_t start_measure, stop_measure;
	time(&start_measure);	
	if(is_send && (snd_filename != NULL)) {
		pthread_create(&threads[0], NULL, (void *)send_data, (void *) &param[0]);
	}

	if(is_recv && (rcv_filename != NULL)) {
		pthread_create(&threads[1], NULL, (void *)receive_data, (void *) &param[1]);
	}

	char result[100];
	char hash[32];
	char hash_final[32];
	int firsttime=0;
	long timer_firsttime;
	double maxspeed=0;
	double minspeed=5000;
	double totalspeed=0;
	int numoftime=0;
	while (stop==0);
}

void processArgs (int argc, char **argv ) {
	char c;

	while ((c = getopt (argc, argv, "i:s:r:h")) != -1)
		switch (c)
	{
		case 'i':
		iface_name = optarg;
		break;
		case 's':
		is_send = 1;
		snd_filename = optarg;
		break;
		case 'r':
		is_recv = 1;
		rcv_filename = optarg;
		break;
		case 'h':
		usage();
		exit(1);
		break;
		case '?':
		if (isprint (optopt))
			printf ("Unknown option `-%c'.\n", optopt);
		else
			printf ("Unknown option character `\\x%x'.\n",
				optopt);
		break;
		default:
		usage();
		exit(1);
	}
	if (!is_send && !is_recv) {
		usage(); exit(1);
	}
}

/*
   Describe usage of this program.
*/
void usage () {
	printf("Usage: ./snd_rcv_file <options>  [file name]\n");
	printf("\nOptions:\n");
	printf("         -i <iface>       : interface name.\n");
	printf("         -s <file name>   : send file to system.\n");
	printf("         -r <file name>   : receive file from system.\n");
	printf("         -h               : help.\n");
}



void send_data(int * value){
	printf("---------------\n");
	/* Process file pointer */
	printf("File name is read: %s\n", snd_filename);
	FILE *fin = fopen(snd_filename,"r");
	if(!fin){
		printf("Can't open the file %s\n", snd_filename);
		goto error_found;
	}
	
	/* Used for processing header of file */
	is_first = 1; 
	
	/* Clear position of file*/
	position = 0;
	filesize = 0;
	send_number = 0;
	
	/* Getting size of file */
	fseek(fin, 0, SEEK_END);
	filesize = ftell(fin);
	printf("Size of read file: %ld\n",filesize);
	rewind(fin);

	while(ftell(fin) != filesize){
    	/* Clear all sending buffer */
		bzero(packet, DMA_BUF_SIZE);

		packet[0] = DMA_PKT_MAGIC_NUMBER_0;
		packet[1] = DMA_PKT_MAGIC_NUMBER_1;
		packet[2] = DMA_PKT_MAGIC_NUMBER_2;
		/* Data of frame to be sent */	             
		pktsize = fread(packet + DMA_PKT_HEADER, 1, DMA_PKT_LEN - DMA_PKT_HEADER, fin);
		
		/*	Include packet length	*/
		for(i = DMA_PKT_HEADER-2; i < DMA_PKT_HEADER; i++)
			packet[i] = ((pktsize*8) >> (8*(DMA_PKT_HEADER-1-i))) & 0xFF;

    	/* Seeking to read next time */
		position = position + pktsize;
		fseek(fin, position, SEEK_SET);
		
    	/* Padding if frame is smaller than 64 */

    	/* Sending packet */  
		int  written_bytes = 0;  

		written_bytes = write(s, packet,pktsize + DMA_PKT_HEADER);
		
		if (written_bytes <= 0) 
		{
			printf("Write data error!\n");
			goto error_found;
		}else{
	//		printf("Write bytes: %d\n",written_bytes);
		}
		
		is_first = 0;
	}
	printf("Send finished\n");
	//sleep(1);	
	stop = 1;
	not_error_found:
	shutdown(s, SHUT_RDWR);
	close(s);
	error = 0;
	return;

	error_found:
	shutdown(s, SHUT_RDWR);
	close(s);
	error = 1;
	return;

}


void receive_data(int* value){  
	
}
