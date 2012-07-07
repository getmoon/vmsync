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

#define KB(n) (1UL << 10)
#define MB(n) (1UL << 20)
#define GB(n) (1UL << 30)

extern int vmsync_file_create(const char * file_name);
extern int vmsync_file_remove(const char * file_name);

extern int vmsync_flock_set(int fd, int code);

#define vmsync_file_lock(fd) vmsync_flock_set(fd, F_WRLCK)
#define vmsync_file_unlock(fd) vmsync_flock_set(fd, F_UNLCK)

#endif

