#ifndef __BASE_H__
#define __BASE_H__

#define FALSE	0
#define TRUE	1

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

#endif

