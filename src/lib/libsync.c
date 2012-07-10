#include "base.h"
#include "fsync.h"

/*
 * Sync APIs - Any problem pls let me known. 
 * E-mail: chengtao786@gmail.com
 */ 

static uint32_t f_block_size;
static int f_lock_fd[LOCK_HASH_SIZE] = {-1, };
char	sync_work_dir[512];

int vmsync_init(const char * work_path , const char * source_file_full_name, int file_id, uint32_t block_size)
{
	int	ret;
	char	file_lock_name[128] = "";
	char	dir_name[128] = "";
	int	fd;
	int	i;

	resource_init();

	sprintf(sync_work_dir , "%s" , work_path);

	if (block_size == 0 || block_size > MB(64))
		return -ERROR_SYNC_FBLOCKLEN;

	if (file_id < 0)
		return -ERROR_SYNC_FID;

	memset(dir_name, 0, 128);
	sprintf(dir_name, "%s/lock/", sync_work_dir);
	if (access(dir_name, F_OK)){
		ret = mkdir(dir_name, 0777);
		if (ret < 0){
			print_error("Create directory %s error" , dir_name);
			return ret;
		}
	}

	memset(dir_name, 0, 128);
	sprintf(dir_name, "%s/send/", sync_work_dir);
	if (access(dir_name, F_OK)){
		ret = mkdir(dir_name, 0777);
		if (ret < 0){
			print_error("Create directory %s error" , dir_name);
			return ret;
		}
	}

	f_block_size = block_size;
	
	for (i = 0; i < LOCK_HASH_SIZE; i++){
		sprintf(file_lock_name, "%s/lock/%d+%d.lck", sync_work_dir , file_id, i);
		f_lock_fd[i] = open(file_lock_name, O_RDWR | O_CREAT, 0666);
		if (f_lock_fd[i] < 0){
			print_error("Create lock file %s error" , file_lock_name);
			return fd;
		}
	}

	sprintf(dir_name, "%s/send/%d/", sync_work_dir, file_id);
	if (access(dir_name, F_OK)){
		ret = mkdir(dir_name, 0777);
		if (ret < 0){
			print_error("Create directory %s error" , dir_name);
			return ret;
		}
	}

	return ERROR_SYNC_OK;
}

void vmsync_fini(const char * source_file_full_name, int file_id)
{
	int	i = 0;
	for (i = 0; i < LOCK_HASH_SIZE; i++)
	if ( f_lock_fd[i] != -1 ){
		close(f_lock_fd[i]);
		f_lock_fd[i] = -1;
	}
	return ;
}

/* /tmp/sync/send/ */
//int vmsync_send(int fild_id, uint64_t offset , uint64_t len)

int vm_file_lock(int fild_id, uint64_t offset , uint64_t len)
{
	uint64_t		i;
	uint64_t		start_blk_id;
	uint64_t		end_blk_id;
	
	start_blk_id = offset / f_block_size;
	len += (offset % f_block_size);

	end_blk_id = start_blk_id + (len / f_block_size);

	/* A complete block-edge */
	if (!(len % f_block_size))
		end_blk_id--;

	for (i = start_blk_id; i <= end_blk_id; i++)
		vmsync_file_lock(f_lock_fd[i%LOCK_HASH_SIZE]);
}

int vm_file_unlock(int fild_id, uint64_t offset , uint64_t len)
{
	uint64_t		i;
	uint64_t		start_blk_id;
	uint64_t		end_blk_id;
	
	start_blk_id = offset / f_block_size;
	len += (offset % f_block_size);

	end_blk_id = start_blk_id + (len / f_block_size);

	/* A complete block-edge */
	if (!(len % f_block_size))
		end_blk_id--;

	for (i = start_blk_id; i <= end_blk_id; i++)
		vmsync_file_unlock(f_lock_fd[i%LOCK_HASH_SIZE]);
}

int vmsync_send(int fild_id, uint64_t offset , uint64_t len)
{
	int			ret;
	uint64_t		i;
	uint64_t		start_blk_id;
	uint64_t		end_blk_id;
	char			file_name[512];

	if (len == 0)
		return ERROR_SYNC_OK;

	start_blk_id = offset / f_block_size;
	len += (offset % f_block_size);

	end_blk_id = start_blk_id + (len / f_block_size);

	/* A complete block-edge */
	if (!(len % f_block_size))
		end_blk_id--;
	
	
	for (i = start_blk_id; i <= end_blk_id; i++){
		sprintf(file_name, "%s/send/%d/all+%llu", sync_work_dir, fild_id, i);
		//print_debug("ready to create file:%s\n", file_name);
		ret = vmsync_file_create(file_name);
	}

	return ret;
}

