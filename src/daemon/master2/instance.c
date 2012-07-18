#define _REENTRANT
#include "base.h"
#include "priv.h"
#include "instance.h"


struct config_instance_t instance_base[INSTANCE_MAX_CNT];


int instance_init(struct config_instance_t * inst)
{
	int			i;

	dowork_lock_init(inst);

	pthread_create(&inst->mana_thread_t, NULL , do_mana_handler , (void*)inst);	
	pthread_create(&inst->dir_thread_t, NULL , do_dir_handler , (void*)inst);	

	for(i = 0; i < MAX_THREAD_PER_INST; i++){
		pthread_create(&inst->work_thread_t[i], NULL , do_work_handler , (void*)inst);	
	}

	return 0;
}


void instance_state_reset(struct config_instance_t * inst)
{
	pthread_mutex_lock(&inst->state_lock);
	inst->state_cnt = 0;	
	pthread_mutex_unlock(&inst->state_lock);
}

void instance_state_inc(struct config_instance_t * inst)
{
	pthread_mutex_lock(&inst->state_lock);
	inst->state_cnt++ ;	
	printf("state cnt %d\n" , inst->state_cnt);
	pthread_mutex_unlock(&inst->state_lock);
}

int instance_state_test(struct config_instance_t * inst)
{
	int		ret;

	pthread_mutex_lock(&inst->state_lock);
	ret = (inst->state_cnt ==  MAX_THREAD_PER_INST);	
	pthread_mutex_unlock(&inst->state_lock);

	return ret;
}
