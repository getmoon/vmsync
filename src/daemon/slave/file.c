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
	int		fd;

	if(msg_type != MSG_TYPE_SYNC_DATA){
		print_info("msg type %d fail\n" , MSG_TYPE_SYNC_DATA);
		return 0;
	}

	pthread_mutex_lock(&cft->filelock);

	fd = open(cft->filename , O_WRONLY |O_CREAT, 0666);
	if(fd < 0){
		print_error("open backup file fail\n");
		pthread_mutex_unlock(&cft->filelock);
		return -1;
	}

	offset = msg_blockid * block_size;
	off_ret = lseek64(fd , offset , SEEK_SET);
	if(off_ret == (off64_t)-1){
		print_error("lseek file fail offset %llu\n" , offset);
		pthread_mutex_unlock(&cft->filelock);
		return -1;
	}

	counter_inc(&rcv_msg_cnt);
	block_id_flag[msg_blockid]++;
#if (0)
	if(counter_read(&rcv_msg_cnt) >= 18760){
		int	i;

		for(i = 0 ; i < 18763 ; i++){
			if(block_id_flag[i] != 1)
				print_debug("zero flag block id %d flag %d\n" , i , block_id_flag[i]);
		}
	}
	//print_debug("rcv block id %d flag %d\n" , counter_read(&rcv_msg_cnt), block_id_flag[msg_blockid]);
#endif
		
	

	//cnt++;
	//print_debug("file_id %lld blockid %d rcv cnt %d\n" , msg_fileid , msg_blockid , cnt);

	ret = write(fd , msg_data , msg_datalen);
	if(ret < 0){
		perror("write fail\n");
		print_error("write file fail \n");
		pthread_mutex_unlock(&cft->filelock);
		return -1;
	}
	close(fd);
	pthread_mutex_unlock(&cft->filelock);
	
	if(ret != msg_datalen){
		print_debug("bug here on write\n");
		exit(0);
	}

	//print_debug("handler block id %d cnt %d\n" , msg_blockid , counter_read(&rcv_msg_cnt));
	
	return 0;
}
