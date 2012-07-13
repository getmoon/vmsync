/*
 * Sync APIs - Any problem pls let me known. 
 * E-mail: chengtao786@gmail.com
 * /tmp/sync/source: vmsync_init(), filename_fileid_blocksize
 * /tmp/sync/send: vmsync_send(), filename_fileid_blocksize_blockid
 * /tmp/sync/fail: vmsync_send() function fails
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

extern int vmsync_init(const char * work_path , const char * source_file_full_name, int file_id, uint32_t block_size);
extern void vmsync_fini(const char * source_file_full_name, int file_id);
extern int vmsync_send(int fild_id, uint64_t offset , uint64_t len);
extern int vmsync_lock(int fild_id, uint64_t offset , uint64_t len);
extern void vmsync_unlock(int fild_id, uint64_t offset , uint64_t len);

#endif

