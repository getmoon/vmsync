#ifndef __SLAVE_PRIV_H__
#define __SLAVE_PRIV_H__

#include "instance.h"


extern int             block_size;
extern int config_init(char * config_file);
extern void * do_mana_handler(void *arg);
extern void * do_work_handler(void *arg);
extern void * do_dir_handler(void * arg);
extern int remote_connect(char * serverip , unsigned short serverport);
extern char            sync_work_dir[512];
extern int             sync_period ;
extern int instance_init(struct config_instance_t * inst);
extern int dowork_lock_init(struct config_instance_t * inst);
extern void instance_state_reset(struct config_instance_t * inst);
extern void instance_state_inc(struct config_instance_t * inst);
extern int instance_state_test(struct config_instance_t * inst);

#endif//__SLAVE_PRIV_H__
