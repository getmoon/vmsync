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

#include "base.h"
#include "priv.h"
#include "instance.h"

int		block_size = DEFAULT_BLOCK_SIZE;
char    	sync_work_dir[512];
int		sync_period = 300;
char		config_file[256];

void usage(void)
{
	printf("./daemon -p {work_path} -t {sync_period} -c {config_file}\n");
}

#if 1
int main(int argc , char ** argv)
{
	int				ret;
	int				i;
	struct config_instance_t *	inst;
	char				dir_name[128];
	
	if( 	argc != 7 ||
		str_unequal(argv[1] , "-p") ||
		file_unexist(argv[2]) ||
		str_unequal(argv[3] , "-t") ||
		isnotnumber(argv[4])  ||
		str_unequal(argv[5] , "-c") 
	){
		usage();
		exit(0);
	}

	resource_init();

	sprintf(sync_work_dir , "%s" , argv[2]);
	sync_period = atoi(argv[4]);
	sprintf(config_file , "%s" , argv[6]);

	memset(dir_name, 0, 128);
	sprintf(dir_name, "%s/lock/", sync_work_dir);
	if (access(dir_name, F_OK)){
	        ret = mkdir(dir_name, 0777);
	        if (ret < 0){
	                print_error("Create directory %s error" , dir_name);
	                return ret;
	        }
	}
	
	memset(dir_name, 0, 128);
	sprintf(dir_name, "%s/send/", sync_work_dir);
	if (access(dir_name, F_OK)){
	        ret = mkdir(dir_name, 0777);
	        if (ret < 0){
	                print_error("Create directory %s error" , dir_name);
	                return ret;
	        }
	}


	ret = config_init(config_file);
	if(ret < 0){
		print_error("config file init fail\n");
		exit(0);
	}

	for(i = 0 ; i < INSTANCE_MAX_CNT ; i++){
		inst = instance_base + i;	
		if(inst->use == 0)
			continue;
		
		instance_init(inst);	
	}

	while(1){
		sleep(3600);
	}

	return(0);
}

#else

int main(int argc , char ** argv)
{
	int				ret;
	int				i;
	struct config_instance_t *	inst;

	ret = config_init("config.txt");
	if(ret < 0){
		print_error("config file init fail\n");
		exit(0);
	}

	config_cft_dump();
	return(0);
}

#endif
