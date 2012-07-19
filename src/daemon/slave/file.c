#define _REENTRANT
#define _LARGEFILE64_SOURCE
#include "base.h"
#include "msg.h"
#include "priv.h"

#define NUM_THREADS 	32

int msg_handler(struct pthread_attr_t * pattr , 
		struct config_file_t * cft , 
		__u32 msg_type , 
		uint64_t msg_fileid , 
		__u32 msg_blockid , 
		__u8 * msg_data , 
		__u32 msg_datalen)
{
	__u64		offset;
	ssize_t		ret;
	off64_t		off_ret;
	//static int	cnt = 0;

	if(msg_type != MSG_TYPE_SYNC_DATA){
		print_info("msg type %d fail\n" , MSG_TYPE_SYNC_DATA);
		return 0;
	}

	offset = msg_blockid * block_size;
	off_ret = lseek64(cft->fd , offset , SEEK_SET);
	if(off_ret == (off64_t)-1){
		print_error("lseek file fail offset %llu\n" , offset);
		return -1;
	}

	//cnt++;
	//print_debug("file_id %lld blockid %d rcv cnt %d\n" , msg_fileid , msg_blockid , cnt);

	ret = write(cft->fd , msg_data , msg_datalen);
	if(ret < 0){
		print_error("write file fail \n");
		return -1;
	}
	
	return 0;
}
