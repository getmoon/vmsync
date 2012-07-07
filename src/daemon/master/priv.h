#ifndef __SLAVE_PRIV_H__
#define __SLAVE_PRIV_H__

#include "instance.h"

#define BACKUP_SVR_PORT		6500

struct pthread_attr_t{
        int             		use;
        pthread_t       		tid;
	struct config_instance_t *	inst;
};        

extern int             block_size;
extern int config_init(char * config_file);
extern void * do_mana_handler(void *arg);
extern void * do_work_handler(void *arg);
extern int remote_connect(char * serverip , unsigned short serverport);

#endif//__SLAVE_PRIV_H__
