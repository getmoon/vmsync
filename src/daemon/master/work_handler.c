#define _REENTRANT
#include "base.h"
#include "msg.h"
#include "fsync.h"
#include "priv.h"
#include "instance.h"

static int filename_parse(char * filename, struct dir_instance_t * elem)
{
	int	i = 0;
	int	tokens = 0;
	char * str = filename;
	char prefix[8] = "";
	char block_id[64] = "";
	char ip_str[64] = "";
	char * curr_str = prefix;

	while (*str){
		if (*str == '+'){
			tokens++;
			if (tokens == 1){
				if (!strcmp(prefix, "all")){
					elem->broadcast = 0xffffffff;
				}else if (!strcmp(prefix, "sgl")){
					;
				}else{
					return -1;
				}
				
				curr_str = block_id;
			}else if (tokens == 2){
				if(parse_u32(block_id, &elem->blockid)){
					return -1;
				}

				curr_str = ip_str;
			}

			i = 0;
			str++;
			continue;
		}

		curr_str[i++] = *str;
		str++;
	}

	
	if (tokens == 1){// parse_blockid
		if(parse_u32(block_id, &elem->blockid))
			return -1;

		return 0;
	}else if (tokens == 2){// //accert this is a signle element, parse_ip
		if (parse_ip(ip_str, &elem->broadcast))
			return -1;

		return 0;
	}

	return -1;
}

static struct dir_instance_t * load_dir(const char * dir_name)
{
	struct dir_instance_t * dir_head = NULL;
	struct dir_instance_t * curr = NULL;
	struct dir_instance_t * prev = NULL;
	DIR 			*sys_dir;
	struct dirent   	*sys_elem;

	sys_dir = opendir(dir_name);
	if (!sys_dir)
		return NULL;

	while ((sys_elem = readdir(sys_dir)) != NULL){
		if (sys_elem->d_type == DT_REG){	// regular files
			curr = (struct dir_instance_t*)malloc(sizeof(struct dir_instance_t));
			sprintf(curr->filename, "%s%s", dir_name, sys_elem->d_name);
			curr->next = NULL;
			if (filename_parse(sys_elem->d_name, curr)){
				return NULL;
			}
			
			if (dir_head == NULL){
				dir_head = prev = curr;
			}else{
				prev->next = curr;
				prev = curr;
			}
		}
	}

	closedir(sys_dir);

	return dir_head;
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

static msg_data_t * load_msg(int fd, __u32 type, __u32 file_id, __u32 blockid, __u32 blk_len, __u8 * msg_buffer)
{
	int	read_size;
	uint64_t offset = (uint64_t)(blockid) * (uint64_t)(blk_len);
	msg_data_t * msg = (msg_data_t*)msg_buffer;

	msg->type = htonl(type);
	msg->fileid = htonl(file_id);
	msg->blockid = htonl(blockid);

	if (lseek64(fd, offset, SEEK_SET) != offset){
		print_error("lseek error\n");
		return NULL;
	}

	read_size = read(fd, msg->data, blk_len);
	msg->datalen = htonl(read_size);

	return msg;
}

static int send_msg(msg_data_t *msg, struct remote_ip_t * rip)
{
	int	ret;

	ret = write(rip->sockfd, /*msg->data*/ (char*)msg, sizeof(msg_data_t) + ntohl(msg->datalen));
	if (ret <= 0)
		return -1;

	return 0;
}

void * do_work_handler(void *arg)
{
	struct config_instance_t * inst = (struct config_instance_t *)arg;
	int					i;
	int					lock_fd[LOCK_HASH_SIZE];
	int					file_id = inst->fileid;
	int					source_file_fid;
	int					ret;
	char					send_dir[512] = "";
	char					file_lock_name[512] = "";
	char					record_signle_name[512] = "";
	struct remote_ip_t			*rip;
	struct dir_instance_t			*dir_head;
	struct dir_instance_t			*dir_curr;
	msg_data_t 				*msg;
	__u8					*msg_buff;
	int					last_handle_time;
	int					curr_handle_time;
	
	signal(SIGPIPE,SIG_IGN);
	
	sprintf(send_dir, "%s/send/%d/", sync_work_dir, file_id);
	if (access(send_dir, F_OK))
		mkdir(send_dir, 0777);

	msg_buff = (__u8*)malloc(MB(16));
	if (!msg_buff){
		print_error("Alloc memory fails\n");
		return NULL;
	}

	for (i = 0; i < LOCK_HASH_SIZE; i++){
		sprintf(file_lock_name, "%s/lock/%d+%d.lck", sync_work_dir, file_id, i);
		lock_fd[i] = open(file_lock_name, O_RDWR | O_CREAT , 0666);	
		if (lock_fd[i] < 0 ) {
			printf("open file fails %s\n", file_lock_name);
			exit(0);
		}
	}

	last_handle_time = get_current_seconds();
	while (1){
		curr_handle_time = get_current_seconds();
		
		if((curr_handle_time - last_handle_time) < sync_period){
			sleep(3);
		}

		last_handle_time = get_current_seconds();
		dir_head = load_dir(send_dir);
		if (!dir_head){			
			usleep(0);
			continue;
		}

		source_file_fid = open(inst->filename, O_RDONLY | O_CREAT, 0666);
		for_each_entry_dir(dir_curr, dir_head){
			vmsync_file_lock(lock_fd[dir_curr->blockid % LOCK_HASH_SIZE]);
			msg = load_msg(source_file_fid, MSG_TYPE_SYNC_DATA, file_id, dir_curr->blockid, block_size, msg_buff);
			if (!msg){
				vmsync_file_unlock(lock_fd[dir_curr->blockid % LOCK_HASH_SIZE]);
				continue;
			}
				
			if (dir_curr->broadcast == 0xffffffff){
				for (i = 0; i < inst->ipcnt; i++){
					rip = inst->remoteip + i;
					if (rip->sockfd != -1) {
						ret = send_msg(msg, rip);
						if (ret < 0){
							rip->sockfd = -1;
							sprintf(record_signle_name, 
								"%s/sgl+%d+%s", send_dir, dir_curr->blockid, rip->ipname);
							vmsync_file_create(record_signle_name);
						}
						vmsync_file_remove(dir_curr->filename);
					}
				}

			}else{
				for (i = 0; i < inst->ipcnt; i++){
					rip = inst->remoteip + i;
					if ((rip->sockfd != -1) && (rip->ip == dir_curr->broadcast) ){
						ret = send_msg(msg, rip);
						if (ret == 0)
							vmsync_file_remove(dir_curr->filename);
					}
				}
			}
			vmsync_file_unlock(lock_fd[dir_curr->blockid % LOCK_HASH_SIZE]);
		}
		close(source_file_fid);
		release_dir(dir_head);
	}

	for (i = 0; i < LOCK_HASH_SIZE; i++)
		close(lock_fd[i]);
	free(msg_buff);


	return 0;
}

