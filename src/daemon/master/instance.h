#ifndef __MASTER_INSTANCE_H__
#define __MASTER_INSTANCE_H__

#include "base.h"

struct remote_ip_t{
	__u32		ip;
	int		sockfd;
};

struct config_instance_t{
        int             	use;
        char            	filename[512];
	__u32			fileid;
	int			ipcnt;
	struct remote_ip_t	remoteip[MAX_REMOTE_IP_CNT];
};





#endif//
