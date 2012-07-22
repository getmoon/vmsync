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
	printf("./finit {src_file}\n");
}


int main(int argc , char ** argv)
{
	char		file[256];
	int		fsize;
	int		ret;
	int		i;
	int		fd;
	__u8		buffer[DEFAULT_BLOCK_SIZE];
	int		block_cnt;
	int		block_last_len;

	if( argc != 2 ){
		usage();
		return 0;
	}

	sprintf(file , "%s" , argv[1]);

	ret = get_file_len(file);
	if(ret < 0){
		print_error("get file %s len fail\n" , file);
		return 0;
	}
	fsize = ret;
	block_cnt = fsize / DEFAULT_BLOCK_SIZE;
	block_last_len = fsize % DEFAULT_BLOCK_SIZE;
	if(block_last_len != 0){
		block_cnt++;
	}

	for(i = 0 ; i < DEFAULT_BLOCK_SIZE ; i++)
		buffer[i] = i&0xff;

        fd = open(file, O_RDWR | O_CREAT);
        if(fd < 0){
                print_error("open source file fail\n");
		exit(0);
        }

	print_debug("file %s size is %d\n" , file , fsize);
	for(i = 0 ; i < block_cnt ; i++){
		if(i == (block_cnt - 1))
			ret = write(fd , buffer , block_last_len);
		else
			ret = write(fd , buffer , DEFAULT_BLOCK_SIZE);
		if(ret < 0){
			print_error("init fail\n");
			exit(0);
		}
	}

	close(fd);

	return 0;
}

