#include "base.h"
#include "fsync.h"

/*
 * Sync APIs - Any problem pls let me known. 
 * E-mail: chengtao786@gmail.com
 */ 

static uint32_t f_block_size;
static int f_lock_fd = -1;

int vmsync_init(const char * source_file_full_name, int file_id, uint32_t block_size)
{
	int	ret;
	char	file_lock_name[128] = "";
	char	dir_name[128] = "";
	int	fd;

	if (block_size == 0 || block_size > MB(64))
		return -ERROR_SYNC_FBLOCKLEN;

	if (file_id < 0)
		return -ERROR_SYNC_FID;

	f_block_size = block_size;
	
	sprintf(file_lock_name, "%s/lock/%d.lck", SYNC_WORK_DIR, file_id);
	fd = open(file_lock_name, O_RDWR | O_CREAT, 0666);
	if (fd < 0){
		print_error("Create lock file %s error" , file_lock_name);
		return fd;
	}

	sprintf(dir_name, "%s/send/%d/", SYNC_WORK_DIR, file_id);
	if (access(dir_name, F_OK)){
		ret = mkdir(dir_name, 0777);
		if (ret < 0){
			print_error("Create directory %s error" , dir_name);
			return ret;
		}
	}
	f_lock_fd = fd;
	return ERROR_SYNC_OK;
}

void vmsync_fini(const char * source_file_full_name, int file_id)
{
	if ( f_lock_fd != -1 ){
		close(f_lock_fd);
		f_lock_fd = -1;
	}
	return ;
}

/* /tmp/sync/send/ */
int vmsync_send(int fild_id, uint64_t offset , uint64_t len)
{
	int			ret;
	uint32_t		i;
	uint32_t		start_blk_id;
	uint32_t		end_blk_id;
	char			file_name[512];

	if (len == 0)
		return ERROR_SYNC_OK;

	start_blk_id = offset / f_block_size;
	len += (offset % f_block_size);

	end_blk_id = start_blk_id + (len / f_block_size);

	/* A complete block-edge */
	if (!(len % f_block_size))
		end_blk_id--;
	
	vmsync_file_lock(f_lock_fd);
	for (i = start_blk_id; i <= end_blk_id; i++){
		sprintf(file_name, "%s/send/%d/all+%llu", SYNC_WORK_DIR, fild_id, i);
		ret = vmsync_file_create(file_name);
	}
	vmsync_file_unlock(f_lock_fd);

	return ret;
}

