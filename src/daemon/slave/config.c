#define _REENTRANT
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "base.h"
#include "priv.h"

#define MAX_CONFIG_FILE_DESC	64

struct config_file_t config_file_desc[MAX_CONFIG_FILE_DESC];

int config_cft_init(void)
{
	int	i;

	for(i = 0 ; i < MAX_CONFIG_FILE_DESC ; i++){
		struct config_file_t *	cft = config_file_desc + i;
		cft->fd = -1;
		cft->use = 0;
		cft->filename[0] = '0';
		cft->id = 0;
	}
	
	return 0;
}

int config_cft_alloc(char * filename , int id)
{
	int			i;
	int			fd;
	struct config_file_t * cft;

	for(i = 0 ; i < MAX_CONFIG_FILE_DESC ; i++){
		cft = config_file_desc + i;
		if(cft->use)
			continue;

		cft->id = id;
		cft->use = 1;
		sprintf(cft->filename , "%s" , filename);

		fd = open(filename , O_RDWR);
		if(fd < 0){
			fprintf(stderr , "ERROR: open backup file %s fail\n" , filename);
			return -1;
		}
		cft->fd = fd;
		return 0;
	}

	fprintf(stderr , "ERROR: too many config line , only support %d\n" , MAX_CONFIG_FILE_DESC);
	return -1;
}

static int config_readline(int fd , char * backup_filename , int * id)
{
	int	cnt;
	char	c;
	int	i = 0;
	int	token_cnt = 0;
	int	token_pos = 0;
	char	linebuffer[1024];
	char	idbuffer[64];

	memset(linebuffer , 0 , 1024);
	memset(idbuffer , 0 , 64);

	while(1){
		cnt = read(fd , &c , 1);
		if(cnt == 0){
			fprintf(stderr , "DEBUG: config read over\n");	
			return 0;
		}

		if(c == '\n'){
			memcpy(idbuffer , linebuffer + token_pos + 1, strlen(linebuffer) - token_pos);
			if(is_number(idbuffer) == 0){
				fprintf(stderr , "ERROR: id in config file is not number\n");
				return -1;
			}
			*id = atoi(idbuffer);
			return 1;
		}

		//fprintf(stderr , "%c " , c);
		if(c == ':'){
			token_cnt++;

			if(token_cnt == 1){
				memcpy(backup_filename , linebuffer , i);
				backup_filename[i] = '\0';
				token_pos = i;
			}

			if(token_cnt > 1){
				fprintf(stderr , "[%s]\n" , linebuffer);
				fprintf(stderr , "ERROR: only support one : in a line\n");
			}
		}
		linebuffer[i++] = c;
	}

	return 0;
}

static int config_file_init(char * config_file)
{
	int		fd;
	int		len;
	char		filename[1024];
	int		id;
	int		ret;

	fd = open(config_file , O_RDONLY);
	if(fd < 0){
		fprintf(stderr , "read config file %s open fail\n" , config_file);
		return -1;
	}

	while(1){
		ret = config_readline(fd , filename , &id);
		if(ret < 0){
			fprintf(stderr , "SUCCESS: parse config file fail\n");
			return 0;
		}

		if(ret == 0)
			break;		
		
		fprintf(stderr , "DEBUG: file=%s id=%d\n" , filename , id);
		ret = config_cft_alloc(filename , id);
		if(ret < 0){
		}
	}

	return 0;
}



int config_init(char * config_file)
{
	int		ret;

	config_cft_init();	

	ret = config_file_init(config_file);
	if(ret < 0){
		return ret;
	}

	return 0;
}
