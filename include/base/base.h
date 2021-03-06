#ifndef __BASE_H__
#define __BASE_H__

#define _LARGEFILE64_SOURCE
#define __USE_FILE_OFFSET64
#define __USE_LARGEFILE64
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <dirent.h>
#include <pthread.h>
#include <signal.h>
#include <sys/sendfile.h>

/*
 *
 */
#define FALSE	0
#define TRUE	1

#define BACKUP_SVR_PORT	9505
#define LOCK_HASH_SIZE 1024
#define DEFAULT_BLOCK_SIZE	4096

typedef unsigned char __u8;
typedef unsigned short  __u16;
typedef unsigned int __u32;
typedef unsigned long long __u64;

static inline int is_number(char *arg)
{
	char *p;
	int len;
	int i;

	if(arg == NULL) {
		return FALSE;
	}

	p = arg;
	while(*p == ' ') {
		p ++;
	}

	len = strlen(p);
	if(len <= 0){
		return FALSE;
	}else if(len == 1){
		if( !isdigit(p[0]) ){
			return FALSE;
		}
	}else if(len == 2){
		if(!isdigit(p[0]) || !isdigit(p[1])){
			return FALSE;
		}
	}else {
		if( (isdigit(p[0]) && isdigit(p[1])) || (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) ){
			for(i = 2; i < len; i ++) {
				if(!isxdigit(p[i])) {
					return FALSE;
				}
			}
		}else{
			return FALSE;
		}
	}

	return TRUE;
}

#define KB(n) (1UL << 10)
#define MB(n) (1UL << 20)
#define GB(n) (1UL << 30)

extern int parse_ip(char * arg, uint32_t * ip);
extern int parse_u32(char * arg, uint32_t * num);
extern int parse_u64(char * arg, uint64_t * num);

extern int vmsync_file_create(const char * file_name);
extern int vmsync_file_remove(const char * file_name);

extern int vmsync_flock_set(int fd, int code);

#define vmsync_file_lock(fd) vmsync_flock_set(fd, F_WRLCK)
#define vmsync_file_unlock(fd) vmsync_flock_set(fd, F_UNLCK)

#define print_debug(fmt , args...)\
	fprintf(stderr , "DEBUG: "fmt , ##args)
#define print_info(fmt , args...)\
	fprintf(stderr , "INFO: "fmt , ##args)
#define print_error(fmt , args...)\
	fprintf(stderr , "ERROR: "fmt , ##args)

#define isnotnumber(arg)	(is_number(arg) == 0)
#define str_equal(str1,str2) ((strlen(str1) == strlen(str2)) && !memcmp( str1 , str2 , strlen(str1)))       
#define str_unequal( str1 , str2 )      ((strlen(str1) != strlen(str2)) || memcmp( str1 , str2 , strlen(str1)))
#define str_in2( str , str0 , str1 )    ( str_equal(str , str0) || str_equal(str , str1) )

static inline int file_exist(char * filename)
{
	if (access(filename, F_OK)){
		return 0;
	}

	return 1;
}

static inline int resource_init()
{
        struct rlimit limit;            
        limit.rlim_cur = 65535;
        limit.rlim_max = 65535;
        setrlimit(RLIMIT_OFILE, &limit);
	return 0;
}

#define file_unexist(filename)	(file_exist(filename)==0)

extern int get_current_seconds(void);
extern int parse_u32(char *arg, uint32_t *num);
extern int parse_ip(char *arg, uint32_t *ip);

typedef struct __counter_t{
	pthread_mutex_t		lock;
	int			cnt;
}counter_t;


static inline int counter_init(counter_t * cnt)
{
	pthread_mutex_init(&cnt->lock , NULL);
	return 0;
}

static inline int counter_inc(counter_t * cnt)
{
	pthread_mutex_lock(&cnt->lock);
	cnt->cnt++;
	pthread_mutex_unlock(&cnt->lock);
	return 0;
}

static inline int counter_add(counter_t * cnt , int value)
{
	pthread_mutex_lock(&cnt->lock);
	cnt->cnt += value;
	pthread_mutex_unlock(&cnt->lock);
	return 0;
}

static inline int counter_read(counter_t * cnt)
{
	int	value;

	pthread_mutex_lock(&cnt->lock);
	value = cnt->cnt;
	pthread_mutex_unlock(&cnt->lock);
	return value;
}

#endif

