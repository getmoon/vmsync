#ifndef __MASTER_INSTANCE_H__
#define __MASTER_INSTANCE_H__

#include "base.h"

#define MAX_REMOTE_IP_CNT	8
#define INSTANCE_MAX_CNT        (64+1)
#define MAX_THREAD_PER_INST	8

struct remote_ip_t{
	__u32			ip;
	int			sockfd[MAX_THREAD_PER_INST];
	char			ipname[64];
};

struct config_instance_t{
        int             	use;
        char            	filename[512];
	uint64_t		fileid;
	int			ipcnt;
	struct remote_ip_t	remoteip[MAX_REMOTE_IP_CNT];
	pthread_t		work_thread_t[MAX_THREAD_PER_INST];
	pthread_mutex_t 	work_thread_lock[MAX_THREAD_PER_INST];
	struct dir_instance_t * work_dir_head[MAX_THREAD_PER_INST];
	pthread_t		mana_thread_t;
	int			lock_fd[LOCK_HASH_SIZE];
};

struct dir_instance_t{
	struct dir_instance_t *next;
	char				filename[512];
	uint32_t			blockid;
	uint32_t			broadcast;
};

#define for_each_entry_dir(dir_c, dir_h)  for (dir_c = dir_h; dir_c != NULL; dir_c = dir_c->next)
	

extern struct config_instance_t instance_base[INSTANCE_MAX_CNT];

#endif//
