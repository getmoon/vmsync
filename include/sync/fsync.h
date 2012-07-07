/*
 * Sync APIs - Any problem pls let me known. 
 * E-mail: chengtao786@gmail.com
 */

#ifndef __FSYNC_H__
#define __FSYNC_H__

extern int vmsync_init(const char * source_file_full_name, int file_id, uint32_t block_size);
extern void vmsync_fini(const char * source_file_full_name, int file_id);

extern int vmsync_send(int fild_id, uint64_t offset , uint64_t len);

#endif

