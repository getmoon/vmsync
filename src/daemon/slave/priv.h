#ifndef __SLAVE_PRIV_H__
#define __SLAVE_PRIV_H__

struct pthread_attr_t{
        int             use;
        pthread_t       tid;
	int		childfd;
};        

extern struct pthread_attr_t * slave_thread_alloc(void);
extern int slave_socket_init(void);
extern void slave_thread_fini(void);

extern int do_accept(int listenfd);
extern void * do_child(void *arg);

#endif//__SLAVE_PRIV_H__
