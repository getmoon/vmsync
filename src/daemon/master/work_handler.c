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

void * do_work_handler(void *arg)
{
	struct config_instance_t * 	inst = (struct config_instance_t *)arg;
	pthread_t			self_id = pthread_self();
	int				work_idx;
	int				i;

	for(i = 0 ; i < MAX_THREAD_PER_INST ; i++){
		if(inst->work_thread_t[i] == self_id){
			work_idx = i;
			print_debug("my work idx is %d\n", work_idx);
		}
	}

	return 0;
}

