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

#define NUM_THREADS 	32

#include "priv.h"


void * do_child(void *arg)
{
	struct pthread_attr_t * pattr = (struct pthread_attr_t *)arg;

	printf("accept child fd %d\n" , pattr->childfd);

	pthread_exit((void *)0);
	return;
}

