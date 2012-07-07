#ifndef __SYNC_MSG_H__
#define __SYNC_MSG_H__

#include "base.h"

enum {
	MSG_TYPE_SYNC_DATA = 1 , 


};

typedef struct __msg_data_t{
	__u32		type;
	__u32		fileid;
	__u32		blockid;
	__u32		datalen;
	__u8		data[0];
}msg_data_t;






#endif
