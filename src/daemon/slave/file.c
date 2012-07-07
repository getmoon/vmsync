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


int msg_handler(struct pthread_attr_t * pattr , 
		struct config_file_t * cft , 
		__u32 msg_type , 
		__u32 msg_fileid , 
		__u32 msg_blockid , 
		__u8 * msg_data , 
		__u32 msg_datalen)
{
	int		offset;
	int		ret;

	if(msg_type != MSG_TYPE_SYNC_DATA){
		print_info("msg type %d fail\n" , MSG_TYPE_SYNC_DATA);
		return 0;
	}

	offset = msg_blockid * block_size;
	ret = lseek(cft->fd , offset , SEEK_SET);
	if(ret < 0){
		print_error("lseek file fail offset %d\n" , offset);
		return -1;
	}

	ret = write(cft->fd , msg_data , msg_datalen);
	if(ret < 0){
		print_error("write file fail \n");
		return -1;
	}
	
	return 0;
}
