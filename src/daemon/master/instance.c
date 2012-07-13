#define _REENTRANT
#include "base.h"
#include "priv.h"
#include "instance.h"


struct config_instance_t instance_base[INSTANCE_MAX_CNT];


int instance_init(struct config_instance_t * inst)
{

	pthread_create(&inst->mana_thread_t, NULL , do_mana_handler , (void*)inst);	
	pthread_create(&inst->work_thread_t, NULL , do_work_handler , (void*)inst);	

	return 0;
}


