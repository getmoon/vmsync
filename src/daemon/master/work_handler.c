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
#include <dirent.h>

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
	DIR 					*sys_dir;
	struct dirent   		*sys_elem;

	sys_dir = opendir(dir_name);
	if (!sys_dir)
		return NULL;

	while (sys_elem = readdir(sys_dir)){
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

static void dump_data(__u8 *data, int size)
{
	int	i;

	printf("[ ");
	for (i = 0; i < size; i++){
		printf("%02X ", data[i]);
	}
	printf("]\n");
}

static msg_data_t * load_msg(int fd, __u32 type, __u32 file_id, __u32 blockid, __u32 blk_len, __u8 * msg_buffer)
{
	int	size = sizeof(msg_data_t) + blk_len;
	int	read_size;
	msg_data_t * msg = (msg_data_t*)msg_buffer;

	msg->type = htonl(type);
	msg->fileid = htonl(file_id);
	msg->blockid = htonl(blockid);

	if (lseek(fd, blockid * blk_len, SEEK_SET) != (blockid * blk_len)){
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

	//print_debug("Send %d bytes to %s\n", msg->datalen, rip->ipname);
	//print_debug("msg->fileid = %d\n", ntohl(msg->fileid));
	//print_debug("msg->blockid = %d\n", ntohl(msg->blockid));
	//print_debug("msg->datalen = %d\n", ntohl(msg->datalen));
	ret = write(rip->sockfd, /*msg->data*/ (char*)msg, sizeof(msg_data_t) + ntohl(msg->datalen));
	if (ret <= 0)
		return -1;

	//dump_data(msg->data, ntohl(msg->datalen) > 64 ? 64 : ntohl(msg->datalen));
	return 0;
}

static void release_msg(msg_data_t *msg)
{
	//if (msg) free(msg);
}

void * do_work_handler(void *arg)
{
	struct config_instance_t * inst = (struct config_instance_t *)arg;
	int					i;
	int					lock_fd;
	int					file_id = inst->fileid;
	char					* file_name = inst->filename;
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
	
	
	sprintf(file_lock_name, "%s/lock/%d.lck", SYNC_WORK_DIR, file_id);
	sprintf(send_dir, "%s/send/%d/", SYNC_WORK_DIR, file_id);
	//source_file_fid = open(inst->filename, O_RDONLY | O_CREAT, 0666);
	//if (source_file_fid < 0){
	//	print_error("open %s error, thread exist\n", inst->filename);
	//	return NULL;
	//}

	if (access(send_dir, F_OK))
		mkdir(send_dir, 0777);

	msg_buff = (__u8*)malloc(MB(16));
	if (!msg_buff){
		print_error("Alloc memory fails\n");
		return NULL;
	}

	lock_fd = open(file_lock_name, O_RDWR | O_CREAT , 0666);	
	while (1){
		vmsync_file_lock(lock_fd);

		dir_head = load_dir(send_dir);
		if (!dir_head){			
			vmsync_file_unlock(lock_fd);
			usleep(0);
			continue;
		}
		source_file_fid = open(inst->filename, O_RDONLY | O_CREAT, 0666);
		for_each_entry_dir(dir_curr, dir_head){
			msg = load_msg(source_file_fid, MSG_TYPE_SYNC_DATA, file_id, dir_curr->blockid, block_size, msg_buff);
			if (!msg){
				continue;
			}
				
#if (00)
			for (i = 0; i < inst->ipcnt; i++){
				rip = inst->remoteip + i;
				if (rip->sockfd != -1 && (dir_curr->broadcast == 0xffffffff || dir_curr->broadcast == rip->ip)){
					ret = send_msg(msg, rip);
					if (ret < 0){
						rip->sockfd = -1;
						sprintf(record_signle_name, "%s/sgl+%d+%s", send_dir, dir_curr->blockid, rip->ipname);
						vmsync_file_create(record_signle_name);
					}
				}
			}

			if ( (dir_curr->broadcast == 0xffffffff) || 
					( (dir_curr->broadcast != 0xffffffff) && (ret == 0))){
				vmsync_file_remove(dir_curr->filename);
			}
#else
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
					}
				}

				vmsync_file_remove(dir_curr->filename);
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
#endif
			//release_msg(msg);
		}
		close(source_file_fid);
		release_dir(dir_head);
		// remove files
		// To All, just delete it, To signal host, if send success, delete it. if NOT, do not delete it.
		vmsync_file_unlock(lock_fd);
	}

	close(lock_fd);
	free(msg_buff);


	return 0;
}
