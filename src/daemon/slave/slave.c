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

int main(int argc , char ** argv)
{
	int 			listenfd;
	int			ret;

	ret = slave_thread_init();
	if(ret < 0){
		fprintf(stderr , "slave thread init fail\n");
		exit(0);
	}	

	listenfd = slave_socket_init();
	if(listenfd < 0){
		exit(0);
	}
	
	do_accept(listenfd);

	return(0);
}

