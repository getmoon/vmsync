#include "base.h"
#include "fsync.h"


/*
extern int vmsync_init(const char * source_file_full_name, int file_id, uint32_t block_size);
extern void vmsync_fini(const char * source_file_full_name, int file_id);
extern int vmsync_send(int fild_id, uint64_t offset , uint64_t len);

extern int vmsync_file_create(const char * file_name);
extern int vmsync_file_remove(const char * file_name);
extern int vmsync_file_lock(const char * file_name);
extern int vmsync_file_unlock(const char * file_name);
*/

int main(int argc, char ** argv)
{
	int	ret;

#if 0
	ret = vmsync_file_create(argv[1]);
	printf("ret = %d\n", ret);

	ret = vmsync_file_remove(argv[1]);
	printf("ret = %d\n", ret);

	//ret = vmsync_file_lock(argv[1]);
	//printf("ret = %d\n", ret);
	//ret = vmsync_file_lock(argv[1]);
	//printf("ret = %d\n", ret);
	//ret = vmsync_file_lock(argv[1]);
	//printf("ret = %d\n", ret);
	fd = open(argv[1], O_RDWR, 0666);
	if (fd < 0)
		perror("open");

	ret = vmsync_file_lock(fd);
	printf("ret = %d\n", ret);
	sleep(10);
	ret = vmsync_file_unlock(fd);
	printf("ret = %d\n", ret);
	while (1);
#endif
	ret = vmsync_init(DEFAULT_SYNC_WORK_DIR , "/opt/test_file", 4, 64);
	printf("vmsync_init ret = %d\n", ret);
	vmsync_send(4, 64, 257);
	vmsync_fini("/opt/test_file", 4);
	
	return 0;
}
