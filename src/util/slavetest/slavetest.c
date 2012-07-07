

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 *	./a.out server_ip source_file file_id block_id size
 */

#include "base.h"
#include "msg.h"

int		sock_fd = -1;
char 		filename[512];
int		file_id;
int		block_id;
int		block_size;
int		file_size;
char		serverip[64];

int socket_init(void)
{
	int sock;                        /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
	unsigned short echoServPort;     /* Echo server port */
	char *servIP;                    /* Server IP address (dotted quad) */
       
	echoServPort = BACKUP_SVR_PORT;
	
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		printf("socket() failed");
		exit(0);
	}

	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
	echoServAddr.sin_family      = AF_INET;             /* Internet address family */
	echoServAddr.sin_addr.s_addr = inet_addr(serverip);   /* Server IP address */
	echoServAddr.sin_port        = htons(echoServPort); /* Server port */

	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0){
		printf("connect() failed");
		exit(0);
	}

	return sock;
}

int get_file_len(char * filename)
{
	FILE *fp;
	int length;  

	fp=fopen(filename ,"rb" ); 
	if(fp == NULL){
		printf("filename is not exist\n");
		exit(0);
	}

	if(fp!=NULL){
		fseek(fp,0L,SEEK_END);
		length=ftell(fp);
		fclose(fp); 
	}

	return length;
}

int arg_init(int argc , char ** argv)
{
	if(argc != 6){
		printf("usage: ./a.out server_ip source_file file_id block_id block_size\n");
		return -1;
	}

	if(is_number(argv[3]) == 0){
		printf("file id must be number\n");
		return -1;
	}

	if(is_number(argv[4]) == 0){
		printf("block id must be number\n");
		return -1;
	}

	if(is_number(argv[5]) == 0){
		printf("size must be number\n");
		return -1;
	}

	sprintf(filename , "%s" , argv[2]);
	sprintf(serverip , "%s" , argv[1]);
	file_id = atoi(argv[3]);
	block_id = atoi(argv[4]);
	block_size = atoi(argv[5]);
	file_size = get_file_len(filename);

	return 0;
}

int main(int argc , char ** argv)
{
	msg_data_t *	msg;
	int		i;
	int		a;
	int		b;
	int		c;
	int		fd;
	int		len;
	int		ret;
	int		slen;
	
	ret = arg_init(argc , argv);
	if(ret < 0){
		return -1;
	}

	sock_fd = socket_init();
	if(sock_fd < 0){
		printf("sock init fail\n");
		exit(0);
	}			

	msg = (msg_data_t*)malloc(sizeof(msg_data_t) + block_size);	
	if(!msg){
		printf("alloc memory fail\n");
	}

	fd = open(filename , O_RDWR);
	if(fd < 0){
		printf("open source file fail\n");
		exit(0);
	}
	

	a = file_size % block_size;
	b = file_size / block_size;
	if( a != 0 ){
		c = b + 1;
	}

	i = 0;	
	while(1){
		len = read(fd , msg->data , block_size);
		if(len == 0){
			printf("read and send over\n");
			break;
		}
		if(len < 0){
			printf("read fail \n");
			break;
		}

		msg->type = htonl(MSG_TYPE_SYNC_DATA);
		msg->fileid = htonl(file_id);
		msg->blockid = htonl(i);	
		msg->datalen = htonl(len);
		slen = sizeof(msg_data_t) + len;
		ret = write(sock_fd , (void*)msg , slen);
		if(ret < 0){
			printf("remote closed\n");
			goto backoff;
		}
		printf("send msg_len %d block %d datalen %d to remote \n" , slen , i , len );
		i++;
	}
	
backoff:
	close(sock_fd);
	close(fd);
	return 0;
}
