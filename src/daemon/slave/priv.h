#ifndef __SLAVE_PRIV_H__
#define __SLAVE_PRIV_H__

#include "msg.h"

struct pthread_attr_t{
        int             use;
        pthread_t       tid;
	int		childfd;
};        

struct config_file_t{
	int		use;
        int             fd;
        char            filename[512];
        unsigned int    id;
};

extern struct pthread_attr_t * slave_thread_alloc(void);
extern int slave_socket_init(void);
extern void slave_thread_fini(void);

extern int do_accept(int listenfd);
extern void * do_child(void *arg);

extern int config_init(char * config_file);
extern int msg_handler(struct pthread_attr_t * pattr , msg_data_t * msg , __u32 datalen);

#endif//__SLAVE_PRIV_H__
