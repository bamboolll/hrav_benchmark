
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
#include <inttypes.h>

#include "hrav_protocol.h"

#undef DEBUG
#ifdef	DEBUG
#define PDEBUG(fmt, args...) printf(fmt, ## args)
#else
#define PDEBUG(fmt, args...)
#endif


#define SEND_BUF_SIZE_DEFAULT 200000
#define SEND_BUF_SIZE_MAX 1000000

#define DEFAULT_IFACE	"nf1"

static int is_send = 0;
static int is_recv = 0;

static char *snd_filename = NULL;
static char *rcv_filename = NULL;
static char *iface_name = DEFAULT_IFACE;

static unsigned long send_number = 0;
static unsigned long recv_number = 0;

int stop = 0;
int i, is_first;
int no_loop = 1;
long config_buffer_size = SEND_BUF_SIZE_DEFAULT;
long buffersize, filesize, datasize, position;
char data_buffer[SEND_BUF_SIZE_MAX];

int sockfd;
int error;

void send_file(int *value);
void receive_data(int *value);
void processArgs (int, char ** );
void usage ();

int main(int argc, char *argv[])
{
	
	struct ifreq ifr;
	struct sockaddr_ll saddr;
	//int sockfd;
	
    
	FILE *fin  = NULL;
	FILE *fout = NULL;
	
	
    /* Argument processing */
	processArgs (argc, argv);
	
#ifdef DEBUG
    sockfd = 1;
#else
	sockfd = open_device(iface_name);
#endif

	pthread_t threads[2];
	int param[2];

	for (i = 0; i < 2; i++) 
		param[i] = i;


	time_t start_measure, stop_measure;
	time(&start_measure);	
	if(is_send && (snd_filename != NULL)) {
		pthread_create(&threads[0], NULL, (void *)send_file, (void *) &param[0]);
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
	while (stop==0) 
		usleep(1000);
}

void processArgs (int argc, char **argv ) {
	char c;

	while ((c = getopt (argc, argv, "i:s:r:h:l:b:")) != -1)
	switch (c)
	{
		case 'i':
		iface_name = optarg;
		break;
		case 'l':
		no_loop = atoi(optarg);
		break;
		case 'b':
		config_buffer_size = atoi(optarg);
		if(config_buffer_size>SEND_BUF_SIZE_MAX)
			config_buffer_size = SEND_BUF_SIZE_MAX;
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
	//printf("         -r <file name>   : receive file from system.\n");
	printf("         -l <n>   : loop n times.\n");
	printf("         -h        : help.\n");
}


void print_current_time_with_ms (void)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    printf("Time ms %ld \n", ms);
    printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",
           (intmax_t)s, ms);
}

double get_time_ms (struct timespec time){
	double tmp;
	tmp = (float)time.tv_sec*1000.0F+ (float) time.tv_nsec / 1000000.0F;
	return tmp;
}


void print_time_with_ms (struct timespec spec)
{
    long            ms; // Milliseconds
    time_t          s;  // Seconds
    //struct timespec spec;

    //clock_gettime(CLOCK_REALTIME, &spec);

    s  = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6); // Convert nanoseconds to milliseconds

    printf("Time ms %ld \n", ms);
    printf("Current time: %"PRIdMAX".%03ld seconds since the Epoch\n",
           (intmax_t)s, ms);
}

// int64_t timespecDiff(struct timespec *timeA_p, struct timespec *timeB_p)
// {
//   return ((timeA_p->tv_sec * 1000000000) + timeA_p->tv_nsec) -
//            ((timeB_p->tv_sec * 1000000000) + timeB_p->tv_nsec);
// }

struct timespec timespecDiff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

void print_timespec(struct timespec time)
{
	printf(" *** TIME *** \n");
	printf(" tv_sec: %ju\n", (uintmax_t)time.tv_sec );
	printf(" tv_nsec: %ju\n", (uintmax_t)time.tv_nsec);
}



void send_file(int * value){
	struct timespec start, end;
	printf("---------------\n");
	/* Process file pointer */
	printf("File name is read: %s\n", snd_filename);
	double send_size = 0;
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
	int bufferID = 0;
	
	/* Getting size of file */
	fseek(fin, 0, SEEK_END);
	filesize = ftell(fin);
	clock_gettime(CLOCK_MONOTONIC, &start);

	printf("Size of file : %ld and loop: %d \n",filesize,no_loop);
	//print_time_with_ms(start);
	print_timespec(start);
	rewind(fin);
	
	while(no_loop >0){
    	/* Clear all sending buffer */
		bzero(data_buffer, config_buffer_size);

		/* Data of frame to be sent */	             
		buffersize = fread(data_buffer, 1, config_buffer_size, fin);
		

    	/* Seeking to read next time */
		position = position + buffersize;
		fseek(fin, position, SEEK_SET);
		
    	/* Padding if frame is smaller than 64 */

    	/* Sending packet */  
		int  written_bytes = 0;  
		int error;
		
#ifdef DEBUG
	    	error = 0;
#else
			error = hrav_send_buff(sockfd, bufferID, data_buffer, buffersize);
#endif
		
		if (error > 0) 
		{
			printf("Sende file error at bufferID %d !!!", bufferID);
			goto error_found;
		}else{
			//printf("Finisheed buffer: %d  - %d\n",bufferID, buffersize);
		}
		
		bufferID ++;
		send_size += buffersize;
		//printf("Current ftell %ld \n", ftell(fin));
		if(ftell(fin) >= filesize){
			no_loop--;
			position = 0;
			rewind(fin);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	printf("************************************************************************\n");
	printf("Send finished no buffer %d\n", bufferID);
	//print_time_with_ms(end);
	print_timespec(end);

	printf("DATA SIZE %f B = %f KB = %f MB\n", send_size, send_size/1024, send_size/(1024*1024));	
	

	struct timespec timeElapsed = timespecDiff(start , end);
	//printf(" Time elapsed : \n");
	//print_timespec(timeElapsed);
	double diff = get_time_ms(timeElapsed); //(double)timeElapsed;
	printf("time elapsed in ms: %f \n", diff);

	double speed = (send_size/1024)/(diff) * 1000;
	printf("Speed %f KBps\n", speed);

	//sleep(1);	
	stop = 1;
	not_error_found:
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	fclose(fin);
	error = 0;
	return;

	error_found:
	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	fclose(fin);
	error = 1;
	return;

}

void receive_data(int* value){  

}