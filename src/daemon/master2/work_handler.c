#define _REENTRANT
#include "base.h"
#include "msg.h"
#include "fsync.h"
#include "priv.h"
#include "instance.h"

int dowork_lock_init(struct config_instance_t * inst)
{
	char                                    file_lock_name[512] = "";
	uint64_t                                file_id = inst->fileid;
	int					i;

        for (i = 0; i < LOCK_HASH_SIZE; i++){
                sprintf(file_lock_name, "%s/lock/%llu+%d.lck", sync_work_dir, file_id, i);
                inst->lock_fd[i] = open(file_lock_name, O_RDWR | O_CREAT , 0666);
                if (inst->lock_fd[i] < 0 ) {
                        print_error("open file fails %s\n", file_lock_name);
                        exit(0);
                }
        }

	return 0;
}

static void release_dir(struct dir_instance_t * head)
{
        struct dir_instance_t * curr = head;
        struct dir_instance_t * next = NULL;

        while(curr){
                next = curr->next;
                free(curr);
                curr = next;
        }
}

static msg_data_t * load_msg(int fd, __u32 type, uint64_t file_id, __u32 blockid, __u32 blk_len, __u8 * msg_buffer)
{
//	int	read_size;
	uint64_t offset = (uint64_t)(blockid) * (uint64_t)(blk_len);
	msg_data_t * msg = (msg_data_t*)msg_buffer;

	msg->type = htonl(type);
	//msg->fileid = htonl(file_id);
	msg->h_fileid = htonl((file_id >> 32) & 0xFFFFFFFFULL);
	msg->l_fileid = htonl(file_id & 0xFFFFFFFFULL);
	msg->blockid = htonl(blockid);

	if (lseek64(fd, offset, SEEK_SET) != offset){
		print_error("lseek error\n");
		return NULL;
	}
#if (0)
	read_size = read(fd, msg->data, blk_len);
#endif
	msg->datalen = htonl(blk_len);

	return msg;
}

static int send_msg(msg_data_t *msg, struct remote_ip_t * rip , int work_idx , int file_fd )
{
	int	ret;
	ret = write(rip->sockfd[work_idx], /*msg->data*/ (char*)msg, sizeof(msg_data_t) );
	if(ret < 0)
		return ret;
	
	ret = sendfile(rip->sockfd[work_idx], file_fd , NULL , ntohl(msg->datalen));
	if(ret < 0)
		return ret;

	return 0;

	//return write(rip->sockfd[work_idx], /*msg->data*/ (char*)msg, sizeof(msg_data_t) + ntohl(msg->datalen));
}

int do_file_sync(struct config_instance_t * inst , 
			int work_idx , __u8 * msg_buff , int file_fd , 
			char * send_dir , 
			struct dir_instance_t * dir_curr)
{
	msg_data_t *		msg;
	int			i;
	struct remote_ip_t *	rip;
	int			ret;
	char                    record_signle_name[512] = "";

	msg = load_msg(file_fd, MSG_TYPE_SYNC_DATA, inst->fileid, dir_curr->blockid, block_size, msg_buff);
	if (!msg){
		print_debug("load msg for block %d fail\n" , dir_curr->blockid);
		return -1;
	}


	if (dir_curr->broadcast == 0xffffffff){
		for (i = 0; i < inst->ipcnt; i++){
			rip = inst->remoteip + i;
			if (rip->sockfd[work_idx] != -1) {
				ret = send_msg(msg, rip , work_idx , file_fd);
				if (ret < 0){
					rip->sockfd[work_idx] = -1;
					sprintf(record_signle_name,
						"%s/sgl+%d+%s", send_dir, dir_curr->blockid, rip->ipname);
					vmsync_file_create(record_signle_name);
				}
				print_debug("file_id %lld blockid %d \n" , inst->fileid , dir_curr->blockid);
			}
		}
		vmsync_file_remove(dir_curr->filename);
	}else{
		for (i = 0; i < inst->ipcnt; i++){
			rip = inst->remoteip + i;
			if ((rip->sockfd[work_idx] != -1) && (rip->ip == dir_curr->broadcast) ){
				ret = send_msg(msg, rip , work_idx , file_fd);
				if (ret == 0)
					vmsync_file_remove(dir_curr->filename);
				print_debug("file_id %lld blockid %d \n" , inst->fileid , dir_curr->blockid);
			}
		}
	}

	//print_debug("work thread %d start to sync block %d\n" , work_idx , dir_curr->blockid);
	return 0;
}

void * do_work_handler(void *arg)
{
	struct config_instance_t * 	inst = (struct config_instance_t *)arg;
	pthread_t			self_id = pthread_self();
	int				work_idx;
	int				i;
	char				send_dir[512] = "";
	uint64_t			file_id = inst->fileid;
	int				source_file_fid;
	__u8 *				msg_buff;
	int				j = 0;

	signal(SIGPIPE,SIG_IGN);

	for(i = 0 ; i < MAX_THREAD_PER_INST ; i++){
		if(inst->work_thread_t[i] == self_id){
			work_idx = i;
			print_debug("my work idx is %d\n", work_idx);
		}
	}

        sprintf(send_dir, "%s/send/%llu/", sync_work_dir, file_id);
        if (access(send_dir, F_OK))
                mkdir(send_dir, 0777);

        msg_buff = (__u8*)malloc(MB(16));
        if (!msg_buff){
                print_error("Alloc memory fails\n");
                return NULL;
        }

	while(1){
		struct dir_instance_t *		dir_head;
		struct dir_instance_t *		dir_curr;

		pthread_mutex_lock(&inst->work_thread_lock[work_idx]);
		while(inst->work_dir_head[work_idx] == NULL){
			pthread_cond_wait(&inst->work_thread_wait[work_idx] , &inst->work_thread_lock[work_idx]);
		}

		dir_head = inst->work_dir_head[work_idx];
		inst->work_dir_head[work_idx] = NULL;

		pthread_mutex_unlock(&inst->work_thread_lock[work_idx]);


                source_file_fid = open(inst->filename, O_RDONLY | O_CREAT, 0666);
                if (source_file_fid < 0){
                        print_error("open %s error\n", inst->filename);
                        release_dir(dir_head);
                        usleep(0);
                        continue;
                }

		for_each_entry_dir(dir_curr, dir_head){
			vmsync_file_lock(inst->lock_fd[dir_curr->blockid % LOCK_HASH_SIZE]);
			do_file_sync(inst , work_idx , msg_buff , source_file_fid , send_dir , dir_curr);
			vmsync_file_unlock(inst->lock_fd[dir_curr->blockid % LOCK_HASH_SIZE]);
			j++;
		}

		close(source_file_fid);
		release_dir(dir_head);	
	}

	free(msg_buff);
	print_debug("total : %d\n" , j);

	return 0;
}


