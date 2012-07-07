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

int		block_size = 4096;

int main(int argc , char ** argv)
{
	int				ret;
	int				i;
	struct config_instance_t *	inst;

	ret = config_init("/home/getmoon/source_config_file");
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

