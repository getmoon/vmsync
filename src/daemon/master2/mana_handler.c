#define _REENTRANT
#include "base.h"
#include "priv.h"
#include "instance.h"


void * do_mana_handler(void *arg)
{
	struct config_instance_t * 	inst = (struct config_instance_t *)arg;
	int				i;
	int				j;
	int				ret;
	struct remote_ip_t *		remote;
	
	signal(SIGPIPE,SIG_IGN);

	for( i = 0 ; i < inst->ipcnt ; i++){
		remote = inst->remoteip + i;
		for(j = 0 ; j < MAX_THREAD_PER_INST ; j++){
			ret = remote_connect(remote->ipname , BACKUP_SVR_PORT);		
			if(ret < 0){
				print_info("connect to %s:%d fail work_thread %d\n" , remote->ipname , BACKUP_SVR_PORT , j);
				remote->sockfd[j] = -1;
				continue;
			}
			remote->sockfd[j] = ret;
			print_debug("connect to %s:%d success for work_thread %d\n" , remote->ipname , BACKUP_SVR_PORT , j);
		}
	}	

	while(1){
		sleep(1);

		for( i = 0 ; i < inst->ipcnt ; i++){
			remote = inst->remoteip + i;
			for(j = 0 ; j < MAX_THREAD_PER_INST ; j++){
				if(remote->sockfd[j] < 0){
					ret = remote_connect(remote->ipname , BACKUP_SVR_PORT);		
					if(ret < 0){
						//print_info("rebuild connect to %s:%d fail for work_thread %d\n" , 
						//		remote->ipname , BACKUP_SVR_PORT , j);
						remote->sockfd[j] = -1;
						continue;
					}
					remote->sockfd[j] = ret;
					print_debug("rebuild connect to %s:%d success for work_thread %d\n" , 
								remote->ipname , BACKUP_SVR_PORT , j);
				}	
			}
		}
	}

	return 0;
}
