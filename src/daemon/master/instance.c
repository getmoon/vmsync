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


#include "priv.h"

#define INSTANCE_MAX_CNT		64

struct config_instance_t instance_base[INSTANCE_MAX_CNT];


int instance_init(struct config_instance_t * inst)
{

	pthread_create(&inst->mana_thread_t, NULL , do_mana_handler , (void*)inst);	
	pthread_create(&inst->work_thread_t, NULL , do_work_handler , (void*)inst);	

	return 0;
}


