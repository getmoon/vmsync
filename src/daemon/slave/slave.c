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

int		block_size = 4096;

int main(int argc , char ** argv)
{
	int 			listenfd;
	int			ret;

	ret = config_init("/home/getmoon/backup_config_file");
	if(ret < 0){
		print_error("config file init fail\n");
		exit(0);
	}

	ret = slave_thread_init();
	if(ret < 0){
		print_error("slave thread init fail\n");
		exit(0);
	}	

	listenfd = slave_socket_init();
	if(listenfd < 0){
		exit(0);
	}
	
	do_accept(listenfd);

	return(0);
}

