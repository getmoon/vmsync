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

int		block_size = DEFAULT_BLOCK_SIZE;
char		config_file[256];

void usage(void)
{
	print_info("./master -c {config_file}");
}

int main(int argc , char ** argv)
{
	int 			listenfd;
	int			ret;

	if(argc != 3 ||
		str_equal(argv[1] , "-c") 
	){
		usage();
		exit(0);
	}

	sprintf(config_file , "%s" , argv[2]);

	ret = config_init(config_file);
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

