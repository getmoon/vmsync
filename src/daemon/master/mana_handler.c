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


void * do_mana_handler(void *arg)
{
	struct config_instance_t * 	inst = (struct config_instance_t *)arg;
	int				i;
	int				ret;
	struct remote_ip_t *		remote;
	
	signal(SIGPIPE,SIG_IGN);

	for( i = 0 ; i < inst->ipcnt ; i++){
		remote = inst->remoteip + i;
		ret = remote_connect(remote->ipname , BACKUP_SVR_PORT);		
		if(ret < 0){
			print_info("connect to %s:%d fail\n" , remote->ipname , BACKUP_SVR_PORT);
			remote->sockfd = -1;
			continue;
		}
		remote->sockfd = ret;
		print_debug("connect to %s:%d success\n" , remote->ipname , BACKUP_SVR_PORT);
	}	

	while(1){
		sleep(1);

		for( i = 0 ; i < inst->ipcnt ; i++){
			remote = inst->remoteip + i;
			if(remote->sockfd < 0){
				ret = remote_connect(remote->ipname , BACKUP_SVR_PORT);		
				if(ret < 0){
					print_info("rebuild connect to %s:%d fail\n" , remote->ipname , BACKUP_SVR_PORT);
					remote->sockfd = -1;
					continue;
				}
				remote->sockfd = ret;
				print_debug("rebuild connect to %s:%d success\n" , remote->ipname , BACKUP_SVR_PORT);
			}	
		}
	}

	return 0;
}
