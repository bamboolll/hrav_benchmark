/****************************************************************
 *	Ho Chi Minh city University of Technology
 *	Faculty of Computer Science and Engineering
 *	
 *	@filename:		serial.h
 *	@date:			Dec 11
 *
 *
 *
 ***************************************************************
 * */
#ifndef _HRAV_PROTOCOL_H_

#define _HRAV_PROTOCOL_H_


#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "stdio.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define DMA_PKT_LEN 		1472
#define MIN_DMA_PKT_LEN		60
#define DMA_PKT_START_LEN	8
#define DMA_PKT_HEADER		8
#define DMA_BUFF_INFO	24
#define	DMA_PKT_MAGIC_NUMBER_0	0xfe
#define DMA_PKT_MAGIC_NUMBER_1	0xca
#define DMA_PKT_MAGIC_NUMBER_2	0xae
#define DMA_BUF_SIZE (DMA_PKT_LEN + 1)
#define DEFAULT_IFACE	"nf1"

#define TRUE 1
#define FALSE 0



struct packet_header{
	char magic[3];
	char type;
	char bufferID[3];
 	char status;
};
typedef struct packet_header HEADER; // 3 + 1 + 3 + 1 = 8 (bytes)

struct send_packet
{
	HEADER header; // 8
	char info[DMA_BUFF_INFO]; // 24
	int length; //length of data: 4
	char data[DMA_BUF_SIZE]; // 1471 : de danh 1 byte NULL cho STRING
};
typedef struct send_packet PACKET; // 8 + 24 + 4 + 1472 = 1508

struct send_buffer
{
	int length; // 4
	char buffer[DMA_BUF_SIZE]; // 1473
};
typedef struct send_buffer BUFFER; // 4 + 1476 = 1480

int open_device(char* iface_name);
int send_data(int sockfd, const char* sendbuff, int len);
void PrintInHex(char *mesg, unsigned char *p, int len);
int hrav_send_buff(int sockfd, int bufferID, char* sendbuff, int send_buffer_length);
int hrav_receive_buff(int sockfd, int* bufferID, unsigned char* receivebuff, int* buffer_length);


#endif


/*	END OF FILE	*/