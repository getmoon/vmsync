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

#include "msg.h"
#include "priv.h"


int msg_handler(struct pthread_attr_t * pattr , msg_data_t * msg , __u32 datalen)
{

	return 0;
}
