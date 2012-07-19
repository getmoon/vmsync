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
	int	read_size;
	uint64_t offset = (uint64_t)(blockid) * (uint64_t)(blk_len);
	msg_data_t * msg = (msg_data_t*)msg_buffer;

	msg->type = htonl(type);
	msg->h_fileid = htonl((file_id >> 32) & 0xFFFFFFFFULL);
	msg->l_fileid = htonl(file_id & 0xFFFFFFFFULL);
	msg->blockid = htonl(blockid);

	if (lseek64(fd, offset, SEEK_SET) != offset){
		print_error("lseek error\n");
		return NULL;
	}
#if (1)
	read_size = read(fd, msg->data, blk_len);
	if(read_size != blk_len){
		print_error("bug here\n");
		printf("read %d len offset %llu\n" , read_size , offset);
		//exit(0);
	}
#endif
	//msg->datalen = htonl(blk_len);
	msg->datalen = htonl(read_size);

	return msg;
}

int			all_load_cnt = 0;
int			all_del_cnt = 0;
int			sgl_create_cnt = 0;
int			sgl_load_cnt = 0;
int			sgl_del_cnt = 0;
int			snd_msg_cnt = 0;

static int send_msg(msg_data_t *msg, struct remote_ip_t * rip , int work_idx , int file_fd )
{
	int	ret;
	int	left_len = 0;

	left_len = sizeof(msg_data_t) + ntohl(msg->datalen);

retry_head:
	ret = write(rip->sockfd[work_idx], /*msg->data*/ (char*)msg, left_len);
	if(ret <= 0){
		perror("write fail\n");
		printf("fd %d\n" , rip->sockfd[work_idx]);
		return ret;
	}

	left_len -= ret;
	if(left_len > 0)
		goto retry_head;
	snd_msg_cnt++;
	return 1;
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
		all_load_cnt++;
		for (i = 0; i < inst->ipcnt; i++){
			rip = inst->remoteip + i;
			if (rip->sockfd[work_idx] != -1) {
				ret = send_msg(msg, rip , work_idx , file_fd);
				if (ret <= 0){
					rip->sockfd[work_idx] = -1;
					sprintf(record_signle_name,
						"%s/sgl+%d+%s", send_dir, dir_curr->blockid, rip->ipname);
					vmsync_file_create(record_signle_name);
					sgl_create_cnt++;
				}
			}else{
				sprintf(record_signle_name,
					"%s/sgl+%d+%s", send_dir, dir_curr->blockid, rip->ipname);
				vmsync_file_create(record_signle_name);
			}
		}
		all_del_cnt++;
		vmsync_file_remove(dir_curr->filename);
	}else{
		sgl_load_cnt++;
		for (i = 0; i < inst->ipcnt; i++){
			rip = inst->remoteip + i;
			if ((rip->sockfd[work_idx] != -1) && (rip->ip == dir_curr->broadcast) ){
				ret = send_msg(msg, rip , work_idx , file_fd);
				if (ret <= 0){
					rip->sockfd[work_idx] = -1;
				}else{
					sgl_del_cnt++;
					vmsync_file_remove(dir_curr->filename);
				}
			}
		}
	}

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
		//inst->work_dir_head[work_idx] = NULL;

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
		}
		print_debug("load all %d load sgl %d del all %d del sgl %d create sgl %d snd_msg %d\n" , 
			all_load_cnt , sgl_load_cnt , all_del_cnt , sgl_del_cnt , sgl_create_cnt , snd_msg_cnt);
		close(source_file_fid);
		release_dir(dir_head);	

		pthread_mutex_lock(&inst->work_thread_lock[work_idx]);
		inst->work_dir_head[work_idx] = NULL;
		pthread_mutex_unlock(&inst->work_thread_lock[work_idx]);
	}

	free(msg_buff);

	return 0;
}


