#define _REENTRANT
#include "base.h"
#include "priv.h"

#define NUM_THREADS 	32

struct pthread_attr_t threads_attr[NUM_THREADS];

struct pthread_attr_t * slave_thread_alloc(void)
{
	int		i;

	for(i = 0 ; i < NUM_THREADS ; i++){
		struct pthread_attr_t * thread = threads_attr + i;		
		if(thread->use == 0){
			memset(thread , 0 , sizeof(struct pthread_attr_t));
			thread->use = 1;
			return thread;
		}
	}

	return NULL;
}

int slave_thread_init(void)
{
	int		i;

	for(i = 0 ; i < NUM_THREADS ; i++){
		struct pthread_attr_t * thread = threads_attr + i;		
		memset(thread , 0 , sizeof(struct pthread_attr_t));
		thread->use = 0;
	}

	return 0;
}

void slave_thread_fini(void)
{

}

