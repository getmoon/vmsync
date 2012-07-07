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
#include "instance.h"

void * do_work_handler(void *arg)
{
	struct config_instance_t * inst = (struct config_instance_t *)arg;

	



	return 0;
}
