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
	for(i = 0; i < MAX_THREAD_PER_INST; i++){
		pthread_create(&inst->work_thread_t[i], NULL , do_work_handler , (void*)inst);	
	}

	return 0;
}


