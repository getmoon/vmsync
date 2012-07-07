#ifndef __BASE_H__
#define __BASE_H__

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>

/*
 *
 */
#define FALSE	0
#define TRUE	1

#define BACKUP_SVR_PORT	9505

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


#endif

