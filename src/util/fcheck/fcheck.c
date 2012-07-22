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
	printf("./fcheck {src_file} {dst_file} \n");
}

int file_load_data(char * file_name , __u64 offset , int size , __u8 * buffer)
{
	int	fd;
	int	ret;

        fd = open(file_name , O_RDONLY);
        if(fd < 0){
                print_error("open source file fail\n");
		exit(0);
        }

	lseek64(fd , offset , SEEK_SET);

	ret = read(fd , (void*)buffer , size);
	if(ret < 0){
		print_error("read file fail\n");
		exit(0);
	}

	close(fd);
	return 0;
}

int file_diff(char * src_file , char * dst_file , int blockid , __u64 offset , int size)
{
	__u8		src_data[DEFAULT_BLOCK_SIZE];
	__u8		dst_data[DEFAULT_BLOCK_SIZE];
	int		i;

	file_load_data(src_file , offset , size , src_data);
	file_load_data(dst_file , offset , size , dst_data);

	for(i = 0 ; i < size ; i++){
		if( src_data[i] != dst_data[i]){
			print_error("diff fail: blockid %d offset %llu , size %d i=%d\n" , 
				blockid , offset , size , i);
			break;
		}
	}

	return 0;
}

int main(int argc , char ** argv)
{
	char		src_file[256];
	char		dst_file[256];
	int		src_fsize;
	int		dst_fsize;
	int		ret;
	int		block_cnt;
	int		block_last_size;
	int		i;

	if( argc != 3 ){
		usage();
		return 0;
	}

	sprintf(src_file , "%s" , argv[1]);
	sprintf(dst_file , "%s" , argv[2]);

	ret = get_file_len(src_file);
	if(ret < 0){
		print_error("get file %s len fail\n" , src_file);
		return 0;
	}
	src_fsize = ret;

	ret = get_file_len(dst_file);
	if(ret < 0){
		print_error("get file %s len fail\n" , src_file);
		return 0;
	}
	dst_fsize = ret;

	if(src_fsize != dst_fsize){
		print_error("file size not equal \n");
		return 0;
	}

	block_cnt = src_fsize / DEFAULT_BLOCK_SIZE;
	block_last_size = src_fsize % DEFAULT_BLOCK_SIZE;
	if(block_last_size != 0){
		block_cnt++;
	}else{
		block_last_size = DEFAULT_BLOCK_SIZE;
	}

	for(i = 0 ; i < block_cnt ; i++){
		if(i == (block_cnt - 1))
			file_diff(src_file , dst_file , i , i*DEFAULT_BLOCK_SIZE , block_last_size);
		else
			file_diff(src_file , dst_file , i , i*DEFAULT_BLOCK_SIZE , DEFAULT_BLOCK_SIZE);

	}

	return 0;
}

