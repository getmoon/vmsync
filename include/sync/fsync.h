/*
 * Sync APIs - Any problem pls let me known. 
 * E-mail: chengtao786@gmail.com
 */

#ifndef __FSYNC_H__
#define __FSYNC_H__

#define DEFAULT_SYNC_WORK_DIR "/tmp/sync/"

enum {
	ERROR_SYNC_OK = 0,
	ERROR_SYNC_FNAMELEN ,
	ERROR_SYNC_FBLOCKLEN ,
	ERROR_SYNC_FID ,
	ERROR_SYNC_INIT ,
};

extern int vmsync_init(const char * work_path , const char * source_file_full_name, uint64_t file_id, uint32_t block_size);
extern void vmsync_fini(const char * source_file_full_name, uint64_t file_id);
extern int vmsync_send(uint64_t fild_id, uint64_t offset, uint64_t len);
extern int vmsync_lock(uint64_t fild_id, uint64_t offset, uint64_t len);
extern void vmsync_unlock(uint64_t fild_id, uint64_t offset, uint64_t len);

#endif

