#ifndef __MASTER_INSTANCE_H__
#define __MASTER_INSTANCE_H__

#include "base.h"

#define MAX_REMOTE_IP_CNT	8
#define INSTANCE_MAX_CNT        64

struct remote_ip_t{
	__u32			ip;
	int			sockfd;
	char			ipname[64];
};

struct config_instance_t{
        int             	use;
        char            	filename[512];
	__u32			fileid;
	int			ipcnt;
	struct remote_ip_t	remoteip[MAX_REMOTE_IP_CNT];
	pthread_t		work_thread_t;
	pthread_t		mana_thread_t;
};


extern struct config_instance_t instance_base[INSTANCE_MAX_CNT];

#endif//
