#define _REENTRANT
#include "base.h"
#include "priv.h"

int		block_size = DEFAULT_BLOCK_SIZE;
char		config_file[256];
counter_t	rcv_msg_cnt;

void usage(void)
{
	printf("./slave -c {config_file}\n");
}

int main(int argc , char ** argv)
{
	int 			listenfd;
	int			ret;

	if(argc != 3 ||
		str_unequal(argv[1] , "-c") 
	){
		usage();
		exit(0);
	}

	counter_init(&rcv_msg_cnt);

	resource_init();
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

