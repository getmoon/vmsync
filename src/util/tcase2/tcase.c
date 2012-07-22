#include "base.h"

/*
 * ./tcase {src_file} {file_id} {block_size}	
 *sudo ./tcase ./test.txt 1 4096 fixed 1
 */
#include "fsync.h"
#include "base.h"
#include "msg.h"

#define str_equal( str1 , str2 )        ((strlen(str1) == strlen(str2)) && !memcmp( str1 , str2 , strlen(str1)))
#define str_unequal( str1 , str2 )      ((strlen(str1) != strlen(str2)) || memcmp( str1 , str2 , strlen(str1)))
#define str_in2( str , str0 , str1 )    ( str_equal(str , str0) || str_equal(str , str1) )

int get_file_len(char * filename)
{
        FILE *fp;
        int length;

        fp=fopen(filename ,"rb" );
        if(fp == NULL){
                printf("filename is not exist\n");
                exit(0);
        }

        if(fp!=NULL){
                fseek(fp,0L,SEEK_END);
                length=ftell(fp);
                fclose(fp);
        }

        return length;
}

void usage(void)
{
	printf("./tcase {src_file} {file_id} {block_size} {\"fixed\"|\"enlarge\"} {loopcnt}\n");
}

char		buffer[64*1024*1024];

int do_sorted_size_test(int fd , uint32_t file_size , uint64_t file_id , uint32_t block_size)
{
	__u64		offset;
	__u64		len = block_size;
	int		i;
	int		ret;

	for(i = 0 ; i < len ; i++){
		buffer[i] = (i + 1)&0xff;
	}

	for(i = 0 ; ; i++){
		offset = block_size * i;
		if(offset >= file_size)
			break;

		if((offset + len) > file_size){
			len = file_size - offset;
		}

		printf("INFO:	file id %llu offset %llu len %llu\n" , file_id , offset , len);	

		lseek64(fd , offset , SEEK_SET);
		signal(SIGINT, SIG_IGN);
		vmsync_lock(0, offset, len);
		ret = write(fd , buffer , len);
		if(ret < 0){
			printf("write file fail\n");
			return 0;
		}
		
		vmsync_send(file_id , offset , len);
		vmsync_unlock(0, offset, len);
		signal(SIGINT, SIG_DFL);

	}

	return 0;
}

int do_fixed_size_test(int fd , uint32_t file_size , uint64_t file_id , uint32_t block_size)
{
	__u64		offset;
	__u64		len = block_size;
	int		ret;
	__u32		maxlen;
	__u32		i;
	

	offset = random() %file_size;
	offset = offset & 0xfffff000; /* additinal*/
	len = random() % block_size;
	maxlen = (file_size - offset);
	len = len % maxlen;

	if((offset + len) > file_size){
		printf("buf here , error offset %lld with len %lld\n" , offset , len);
		return 0;
	}

	for(i = 0 ; i < len ; i++){
		buffer[i] = random()&0xff;
	}

	lseek64(fd , offset , SEEK_SET);
	signal(SIGINT, SIG_IGN);
	vmsync_lock(0, offset, len);
	ret = write(fd , buffer , len);
	if(ret < 0){
		printf("write file fail\n");
		return 0;
	}
	
	printf("INFO:	file id %llu offset %llu len %llu\n" , file_id , offset , len);	
	vmsync_send(file_id , offset , len);
	vmsync_unlock(0, offset, len);
	signal(SIGINT, SIG_DFL);
	return 0;
}

int do_enlarge_size_test(int fd , __u32 file_size , uint64_t file_id , __u32 block_size)
{
	do_fixed_size_test(fd , file_size , file_id , block_size);
	return 0;
}

int main(int argc , char ** argv)
{
	char		file_name[256];
	uint64_t	file_id;
	__u32		file_size;
	__u32		block_size;
	char		operate[32];
	int		ret;
	int		fd;
	int		loopcnt;
	int		i = 0;
	int		time_seconds;

	if(	(argc != 6) ||
		(is_number(argv[2]) == 0) ||
		(is_number(argv[3]) == 0) ||
		(str_unequal(argv[4] , "fixed") && str_unequal(argv[4] , "enlarge") && str_unequal(argv[4] , "sorted")) ||
		(is_number(argv[5]) == 0) 
		){
		usage();
		return 0;
	}
		
	time_seconds = get_current_seconds();
	printf("current seconds %d\n" , time_seconds);
//	sleep(61);
//	time_seconds = get_current_seconds();
//	printf("current seconds %d\n" , time_seconds);
	

	sprintf(file_name , "%s" , argv[1]);
	file_id = strtoll(argv[2], NULL, 0);
	block_size = DEFAULT_BLOCK_SIZE;
	file_size = get_file_len(file_name);
	sprintf(operate , "%s" , argv[4]);
	loopcnt = atoi(argv[5]);

	ret = vmsync_init(DEFAULT_SYNC_WORK_DIR , file_name , file_id , block_size);
	if(ret < 0){
		printf("init vmsync library fail\n");
		return 0;
	}

	printf("file_name = %s\n", file_name);
        fd = open(file_name , O_RDWR);
        if(fd < 0){
                printf("open source file fail\n");
                exit(0);
        }

	srandom(time(NULL));

	if(str_equal(operate , "enlarge"))
		file_size += (10*1024*1024);	
		

	if(loopcnt == 0){
		while(1){
			printf("start up %d times test\n" , i);
			if(str_equal(operate , "fixed")){
				do_fixed_size_test(fd , file_size , file_id , block_size);
			}else if(str_equal(operate , "enlarge")){
				do_enlarge_size_test(fd , file_size , file_id , block_size);
			}else{
				printf("error here\n");
			}
			i++;
			usleep(100);
		}

	}else{
		for(i = 0 ; i < loopcnt ; i++){
			printf("start up %d times test\n" , i);
			if(str_equal(operate , "fixed")){
				do_fixed_size_test(fd , file_size , file_id , block_size);
			}else if(str_equal(operate , "enlarge")){
				do_enlarge_size_test(fd , file_size , file_id , block_size);
			}else if(str_equal(operate , "sorted")){
				do_sorted_size_test(fd , file_size , file_id , block_size);	
			}else{
				printf("error here\n");
			}
			usleep(100);
		}
	}
		
	
	printf("success %d test case execut\n" , i);
	

	close(fd);

	vmsync_fini(file_name , file_id);
	return 0;
}





