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
        uint64_t	id;
	pthread_mutex_t filelock;
};

extern struct pthread_attr_t * slave_thread_alloc(void);
extern int slave_socket_init(void);
extern void slave_thread_fini(void);
extern int slave_thread_init(void);
extern void slave_thread_fini(void);

extern int do_accept(int listenfd);
extern void * do_child(void *arg);

extern int config_init(char * config_file);
extern struct config_file_t * config_cft_lookup(uint64_t file_id);

extern int msg_handler(struct pthread_attr_t * pattr ,
                struct config_file_t * cft ,
                __u32 msg_type ,
                uint64_t msg_fileid ,
                __u32 msg_blockid ,
		__u8 * msg_data , 
                __u32 msg_datalen);


extern int             block_size;
extern counter_t       rcv_msg_cnt;



#endif//__SLAVE_PRIV_H__
