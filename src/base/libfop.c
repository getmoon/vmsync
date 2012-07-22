#include "base.h"

int vmsync_flock_set(int fd, int code)
{
	struct flock lock;

	memset(&lock,0,sizeof(struct flock));
	lock.l_start=SEEK_SET;
	lock.l_whence=0;
	lock.l_len=0;
	lock.l_type = code;

	return fcntl(fd, F_SETLKW, &lock);
}

int vmsync_file_create(const char * file_name)
{
	int	fd;

	if (access(file_name, F_OK)){
		fd = open(file_name, O_RDWR | O_CREAT, 0666);
		if (fd < 0){
			perror("Create file error");
			return fd;
		}
		close(fd);
	}
	return 0;
}

int vmsync_file_remove(const char * file_name)
{
	int ret = 0;

	if (!access(file_name, F_OK)){
		ret = remove(file_name);
		if (ret < 0)
			perror("Delete file error");
	}

	return ret;
}

