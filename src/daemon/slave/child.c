#define _REENTRANT
#include "base.h"
#include "msg.h"
#include "priv.h"
#define NUM_THREADS 	32


#define ALLOC_BUF_SIZE	(64 * 1024 * 1024)



void * do_child(void *arg)
{
	struct pthread_attr_t * pattr = (struct pthread_attr_t *)arg;
	int			len;
	int			offset;
	msg_data_t *		msg;
	__u32			msg_type;
	uint64_t		msg_fileid;
	uint64_t		msg_fileid_l;
	uint64_t		msg_fileid_h;
	__u32			msg_blockid;
	__u32			msg_datalen;
	__u32			msg_totlen;
	struct config_file_t *	cft = NULL;
	int			cnt = 0;

	msg = (msg_data_t *)malloc( ALLOC_BUF_SIZE );
	if(!msg){
		print_error("child thread alloc buffer fail\n");
		goto backoff;
	}

	print_debug("accept child fd %d\n" , pattr->childfd);
	
	offset = 0;
	while(1){
		//print_debug("start read child\n");
		len = read(pattr->childfd , (void*)msg + offset , ALLOC_BUF_SIZE/2);
		if(len < 0){
			print_debug("read fail happen\n");
			goto backoff;
		}

		if(len == 0){
			print_info("remote close socket happen\n");
			goto backoff;
		}

next_msg_test:
		offset += len;
		if(offset < sizeof(msg_data_t)){
			//print_debug("not enough for msg head, offset = %d len %d\n", offset , len);
			continue;
		}
		
		msg_type = ntohl(msg->type);
		//msg_fileid = ntohl(msg->fileid);
		msg_blockid = ntohl(msg->blockid);
		msg_datalen = ntohl(msg->datalen);
		msg_fileid_l = (uint64_t)ntohl(msg->l_fileid);
		msg_fileid_h = (uint64_t)ntohl(msg->h_fileid);
		msg_fileid = (msg_fileid_l & 0xFFFFFFFF) + ((msg_fileid_h & 0xFFFFFFFF) << 32);

		if(offset < (sizeof(msg_data_t) + msg_datalen)){
			//print_debug("not enough for msg head and data len %d type %d offset %d len %d\n" , msg_datalen , msg_type , offset , len);
			continue;
		}

		msg_totlen = sizeof(msg_data_t) + msg_datalen;		

		if(cft == NULL){
			cft = config_cft_lookup(msg_fileid);
			if(cft == NULL){
				print_info("no config exist for file id %llu\n" , msg_fileid);
				goto backoff;
			}
		}

		//print_debug("enough for a msg totlen %d datalen %d offset %d len %d\n" , msg_totlen , msg_datalen , offset , len);
		cnt++;	
		//print_debug("work thread %d msg cnt %d\n" , (int)pattr , cnt);
		msg_handler(pattr , cft , msg_type , msg_fileid , msg_blockid , msg->data , msg_datalen);

		offset -= msg_totlen;
		if(offset > 0){
			memcpy((void*)msg , (void*)msg + msg_totlen , offset);
			len = 0;
			goto next_msg_test;
		}
	}

backoff:
	close(pattr->childfd);
	pthread_exit((void *)0);
	return NULL;
}

