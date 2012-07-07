#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_THREADS 	32

#include "msg.h"
#include "priv.h"

#define ALLOC_BUF_SIZE	(64 * 1024 * 1024)

int do_enough_read(int fd , char * buffer , int req_len)
{
	int	len;
	int	left_len = req_len;
	char *	ptr = buffer;

retry:
	len = read(fd , ptr , left_len);
	if(len == 0){
		goto retry;
	}

	if(len < 0){
		fprintf(stderr , "INFO: child socket closed \n");
		return -1;
	}

	left_len -= len;
	ptr += len;

	if(left_len == 0)
		return req_len;

	goto retry;
} 


void * do_child(void *arg)
{
	struct pthread_attr_t * pattr = (struct pthread_attr_t *)arg;
	int			len;
	int			offset;
	msg_data_t *		msg;
	__u32			msg_type;
	__u32			msg_fileid;
	__u32			msg_blockid;
	__u32			msg_datalen;

	msg = (msg_data_t *)malloc( ALLOC_BUF_SIZE );
	if(!msg){
		fprintf(stderr , "ERROR: child thread alloc buffer fail\n");
		goto backoff;
	}

	fprintf(stderr , "accept child fd %d\n" , pattr->childfd);
	
	offset = 0;
	while(1){

		len = read(pattr->childfd , (void*)msg + offset , ALLOC_BUF_SIZE/2);
		if(len < 0){
			fprintf(stderr , "DEBUG: read fail happen\n");
			goto backoff;
		}

		if(len == 0){
			fprintf(stderr , "INFO: read len happen\n");
			continue;
		}
		offset += len;
		if(offset < sizeof(msg_data_t)){
			fprintf(stderr , "DEBUG: not enough for msg head\n");
			continue;
		}
		
		msg_type = ntohl(msg->type);
		msg_fileid = ntohl(msg->fileid);
		msg_blockid = ntohl(msg->blockid);
		msg_datalen = ntohl(msg->datalen);

		if(offset < (sizeof(msg_data_t) + msg_datalen)){
			fprintf(stderr , "DEBUG: not enough for msg head and data\n");
			continue;
		}
		

		fprintf(stderr , "read len %d\n" , len);


		fprintf(stderr , "DEBUG: msg type %d file id %d block id %d datalen %d\n" , 
			msg_type , msg_fileid , msg_blockid , msg_datalen);
#if (00)
		len = do_enough_read(pattr->childfd , (void*)msg , sizeof(msg_data_t));
		if(len != sizeof(msg_data_t)){
			fprintf(stderr , "BUG: not enough data for msg_data_t\n");
			goto backoff;
		}

		fprintf(stderr , "DEBUG: msg type %d file id %d block id %d datalen %d\n" , 
			msg->type , msg->fileid , msg->blockid , msg->datalen);
		
		
		len = do_enough_read(pattr->childfd , msg->data , msg->datalen);
		if(len < 0){
			fprintf(stderr , "DEBUG: enough read fail on data\n");
			goto backoff;
		}

		if(len != msg->datalen){
			fprintf(stderr , "BUG: not enough data for msg_data_t\n");
			goto backoff;
		}

		fprintf(stderr , "DEBUG: success read a msg\n");
#endif
	}

backoff:
	pthread_exit((void *)0);
	return;
}

