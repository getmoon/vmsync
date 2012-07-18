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

static int load_dir(struct config_instance_t * inst , const char * dir_name)
{
        struct dir_instance_t * curr = NULL;
        DIR                     *sys_dir;
        struct dirent           *sys_elem;
        int                     i = 0;
	int			work_idx = 0;
	static int		cnt = 0;

        sys_dir = opendir(dir_name);
        if (!sys_dir)
                return 0;

        while ((sys_elem = readdir(sys_dir)) != NULL){
                if (sys_elem->d_type == DT_REG){        // regular files
                        curr = (struct dir_instance_t*)malloc(sizeof(struct dir_instance_t));
                        if(curr == NULL){
                                print_error("alloc memory fail\n");
                                goto backoff;
                        }

                        sprintf(curr->filename, "%s%s", dir_name, sys_elem->d_name);
                        curr->next = NULL;
                        if (filename_parse(sys_elem->d_name, curr)){
                                free(curr);
                                continue;
                        }
			cnt++;

			print_debug("load block id %d load cnt %d\n" , curr->blockid , cnt);

			work_idx = i%MAX_THREAD_PER_INST;

			pthread_mutex_lock(&inst->work_thread_lock[work_idx]);
			//print_debug("add blockid %d task to work_idx %d \n" , curr->blockid , work_idx);
			curr->next = inst->work_dir_head[work_idx];	
			inst->work_dir_head[work_idx] = curr;
			pthread_mutex_unlock(&inst->work_thread_lock[work_idx]);

                	i++;
                	if(i >= 2048)
                       		break;
		}
        }
backoff:
        closedir(sys_dir);

        return 0;
}

void * do_dir_handler(void * arg)
{
	struct config_instance_t * 	inst = (struct config_instance_t *)arg;
	char                            send_dir[512] = "";
	uint64_t                        file_id = inst->fileid;
        int                             last_handle_time;
        int                             curr_handle_time;
	int				i;
	int				need_wait;

	signal(SIGPIPE,SIG_IGN);

        sprintf(send_dir, "%s/send/%llu/", sync_work_dir, file_id);
        if (access(send_dir, F_OK))
                mkdir(send_dir, 0777);


	last_handle_time = get_current_seconds();
	while(1){
                curr_handle_time = get_current_seconds();
                if((curr_handle_time - last_handle_time) < sync_period){
                        usleep(10000);
			continue;
                }
		last_handle_time = get_current_seconds();

		load_dir(inst , send_dir);

		for(i = 0 ; i < MAX_THREAD_PER_INST ; i++)
			pthread_cond_signal(&inst->work_thread_wait[i]);

		need_wait = 0;
		while(1){
			for(i = 0 ; i < MAX_THREAD_PER_INST ; i++){
				pthread_mutex_lock(&inst->work_thread_lock[i]);
				if(inst->work_dir_head[i] != NULL)
					need_wait = 1;
				pthread_mutex_unlock(&inst->work_thread_lock[i]);
			}

			if(need_wait == 0)
				break;
			
			usleep(1000);
			need_wait = 0;
		}
		print_debug("test out\n");
	}
        return 0;
}
