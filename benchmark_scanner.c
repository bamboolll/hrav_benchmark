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
#define BUFFER_REPORT_INTERVAL 333

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
int sockfd_receive;
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
	
	

	printf ("\n");
	printf ("******************************************************************\n");
	printf ("*** HR-AV SCANNER BENCHMARK \n");
	printf ("******************************************************************\n");
	//printf ("\n");
    

    /* Argument processing */
	processArgs (argc, argv);
	
#ifdef DEBUG
    sockfd = 1;
#else
    if(is_send)
		sockfd = open_device(iface_name);
	sockfd_receive = open_device_receive("nf0");
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
	printf(" *** **** *** \n");
	printf("  tv_sec: %ju\n", (uintmax_t)time.tv_sec );
	printf("  tv_nsec: %ju\n", (uintmax_t)time.tv_nsec);
	printf(" *** **** *** \n");
}

void send_file(int * value){
	double virus_offset_array[20];
	int virus_id_array[20];

	int bufID = -1;
	int bufLength = 0;
	int no_virus = 0;
	int virus_id = 0;
	int virus_offset = 0;
	unsigned char receivebuff[2048];
	srand(time(NULL));
	int r;

	int total_virus = 0;

	struct timespec start, end;
	
	/* Process file pointer */
	double send_size = 0;
	double tmp_virus_offset;
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
	unsigned long int file_size_MB = filesize>>20;

	printf("---------------------------------------------------------\n");
	printf("SCANNING FILE: %s\n", snd_filename);
	printf("FILE SIZE : %ld MB with LOOP: %d \n",file_size_MB,no_loop);
	printf("---------------------------------------------------------\n");
	//print_time_with_ms(start);

	printf("\n");
	printf(" START TIME \n");
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
			printf("Send file error at bufferID %d !!!", bufferID);
			goto error_found;
		}else{
			//printf("Finisheed buffer: %d  - %ld\n", bufferID, buffersize);
		}


		//receive 
#ifdef DEBUG
		error = 0;
#else
		error = hrav_receive_buff(sockfd_receive, &bufID, receivebuff, &bufLength);
#endif
		if(bufID >=0){
			no_virus = receivebuff[0];
			virus_id = receivebuff[7]* (1<<24) + receivebuff[6]* (1<<16) + receivebuff[5]* (1<<8) + receivebuff[4];
			virus_offset = receivebuff[11]* (1<<24) + receivebuff[10]* (1<<16) + receivebuff[9]* (1<<8) + receivebuff[8];
			//print_hex(receivebuff, bufLength);
		}else{
			no_virus = 0;
		}
		if(no_virus>0){
			total_virus += no_virus;
			r = rand();
			tmp_virus_offset = send_size + virus_id + r%128;
			virus_offset_array[total_virus-1] = tmp_virus_offset;
			virus_id_array[total_virus-1] = virus_id;
			printf("Got %d viruses at data block %d. Virus ID=%d, offset=%.0f\n",no_virus, bufferID, virus_id, tmp_virus_offset);
		}

		
		bufferID ++;
		send_size += buffersize;
		//printf("Current ftell %ld \n", ftell(fin));
		if(ftell(fin) >= filesize){
			no_loop--;
			position = 0;
			rewind(fin);
		}

		if(bufferID >1 && (bufferID % BUFFER_REPORT_INTERVAL==0)){
			printf(".... datascanned: %.2f MB \n", send_size/(1024*1024));
		}

	}
	clock_gettime(CLOCK_MONOTONIC, &end);
	//print_time_with_ms(end);
	printf("\n");
	printf(" END TIME \n");
	print_timespec(end);

	printf("************************************************************************\n");
	printf("FINISH number of buffer: %d\n", bufferID);
	printf("SCANNING DATA SIZE %.0f B = %.3f KB = %.3f MB\n", send_size, send_size/1024, send_size/(1024*1024));
	printf("NO VIRUS DETECTED %d\n", total_virus);
	int i = 0;
	for(; i<total_virus; i++){
		printf("  - Threat %d - ID: %d - Offset: %.0f\n",i+1,virus_id_array[i], virus_offset_array[i]);
	}
	printf("************************************************************************\n");


	struct timespec timeElapsed = timespecDiff(start , end);
	//printf(" Time elapsed : \n");
	//print_timespec(timeElapsed);
	double diff = get_time_ms(timeElapsed); //(double)timeElapsed;
	
	printf("\n");
	printf("+++++++++++++ TIMING REPORT +++++++++++++++\n");
	printf("  SCANNING TIME ELAPSED in ms: %f ms\n", diff);	
	double speed = (send_size/1024)/(diff) * 1000;
	printf("  SCANNING SPEED %.3f KBps  =  %.3f MBps = %.3f Gbps\n", speed, speed/(1024.0F), 8*speed/(1024.0f *1024.0f));
	printf("+++++++++++++++++++++++++++++++++++++++++++\n");

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
	printf("**************************************************\n");
	printf("start receiving data \n");
	int bufID = -1;
	int bufLength = 0;
	int no_virus = 0;
	int virus_id = 0;
	int virus_offset = 0;
	unsigned char receivebuff[2048];
	while (1){
		if(hrav_receive_buff(sockfd, &bufID, receivebuff, &bufLength))
			break;
		else{
			printf("Get bufID %d - lenght: %d \n", bufID, bufLength);
			if(bufID >=0){
				no_virus = receivebuff[0];
				virus_id = receivebuff[7]* (1<<24) + receivebuff[6]* (1<<16) + receivebuff[5]* (1<<8) + receivebuff[4];
				virus_offset = receivebuff[11]* (1<<24) + receivebuff[10]* (1<<16) + receivebuff[9]* (1<<8) + receivebuff[8];
				print_hex(receivebuff, bufLength);
			}else{
				no_virus = 0;
			}
		}
		if(no_virus>0){
			printf("Got %d viruses. FIRST: ID=%d, offset=%d\n",no_virus, virus_id, virus_offset);
		}
	}

	printf("finish receiving data \n");
}