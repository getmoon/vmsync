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
	__u32			msg_totlen;

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
			fprintf(stderr , "INFO: remote close socket happen\n");
			goto backoff;
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

		msg_totlen = sizeof(msg_data_t) + msg_datalen;		
		fprintf(stderr , "DEBUG: enough for a msg totlen %d\n" , msg_totlen);

		msg_handler(pattr , msg , msg_datalen);

		offset -= msg_totlen;
		if(offset > 0){
			memcpy((void*)msg , (void*)msg + msg_totlen , offset);
		}
	}

backoff:
	pthread_exit((void *)0);
	return;
}

