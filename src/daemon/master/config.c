#define _REENTRANT
#include "base.h"
#include "priv.h"


int config_cft_init(void)
{
	int				i;
	int				j;
	struct remote_ip_t		*rip;
	struct config_instance_t	*inst;

	for(i = 0 ; i < INSTANCE_MAX_CNT ; i++){
		inst = instance_base + i;
		inst->use = 0;
		//inst->filename[0] = '0';
		inst->fileid = 0;
		inst->ipcnt = 0;

		memset(inst->filename, 0, 512);
		for (j = 0; j < MAX_REMOTE_IP_CNT; j++){
			rip = inst->remoteip + j;
			rip->sockfd = -1;
			rip->ip = 0;
			//sprintf(rip->ipname, "%d.%d.%d.%d", 0, 0, 0, 0);
			memset(rip->ipname, 0, 64);
		}
	}

	return 0;
}

int config_cft_dump(void)
{
	int				i;
	int				j;
	struct remote_ip_t		*rip;
	struct config_instance_t	*inst;

	for(i = 0 ; i < INSTANCE_MAX_CNT ; i++){
		inst = instance_base + i;
		if (inst->use == 0)
			continue;
		
		print_debug("[%d][use=%d][filename=%s][fileid=%d][ipcnt=%d]", i, inst->use, inst->filename, inst->fileid, inst->ipcnt);
		for (j = 0; j < inst->ipcnt; j++){
			rip = inst->remoteip + j;
			print_debug("[%08x - %s]", rip->ip, rip->ipname);
		}
		print_debug("\n");
	}

	return 0;
}

struct config_instance_t *  config_cft_inst_alloc()
{
	int			i;
	struct config_instance_t * inst;

	for(i = 0 ; i < INSTANCE_MAX_CNT ; i++){
		inst = instance_base + i;
		if (inst->use)
			continue;

		inst->use = 1;
		return inst;
	}

	print_error("too many config line , only support %d\n" , INSTANCE_MAX_CNT - 1);
	return NULL;
}

static int config_readline(int fd , struct config_instance_t * inst)
{
	int	cnt;
	char	c;
	int	i = 0;
	int	token_cnt = 0;
	int	token_pos = 0;

	char	linebuffer[1024];
	char	idbuffer[64];
	int	ip_idx = 0;

	memset(linebuffer , 0 , 1024);
	memset(idbuffer , 0 , 64);

	while(1){
		// source_file_full_name_1:id_1:IP1:IP2:IP3..
		cnt = read(fd , &c , 1);
		if(cnt == 0){
			print_debug("config read over\n");	
			return 0;
		}

		// IP
		if( c == '\n'){
			memcpy(inst->remoteip[ip_idx].ipname, linebuffer + token_pos + 1, strlen(linebuffer) - token_pos);
			inst->ipcnt++;
			if (parse_ip(inst->remoteip[ip_idx].ipname, &inst->remoteip[ip_idx].ip)){
				fprintf(stderr , "ERROR:%s is not a valid ip addr\n", inst->remoteip[ip_idx].ipname);
				return -1;
			}
			
			return 1;
		}

		if(c == ':'){
			token_cnt++;

			if(token_cnt == 1){
				memcpy(inst->filename , linebuffer , i);
				inst->filename[i] = '\0';
				token_pos = i;
			}

			if (token_cnt == 2){
				memcpy(idbuffer , linebuffer + token_pos + 1, strlen(linebuffer) - token_pos);
				if(is_number(idbuffer) == 0){
					fprintf(stderr , "ERROR: id in config file is not number\n");
					return -1;
				}

				idbuffer[strlen(linebuffer) - token_pos] = '\0';
				inst->fileid = atoi(idbuffer);
				token_pos = i;
			}

			// parse_ip
			if (token_cnt > MAX_REMOTE_IP_CNT + 1){
				fprintf(stderr , "[%s]\n" , linebuffer);
				fprintf(stderr , "ERROR: only support %d IP : in one inst\n", MAX_REMOTE_IP_CNT);
				return -1;
			}

			if(token_cnt > 2){
				memcpy(inst->remoteip[ip_idx].ipname, linebuffer + token_pos + 1, strlen(linebuffer) - token_pos);
				if (parse_ip(inst->remoteip[ip_idx].ipname, &inst->remoteip[ip_idx].ip)){
					fprintf(stderr , "ERROR: %s is not a valid ip addr\n", inst->remoteip[ip_idx].ipname);
					return -1;
				}

				inst->remoteip[ip_idx].ipname[strlen(linebuffer) - token_pos] = '\0';
				token_pos = i;
				inst->ipcnt++;
				ip_idx++;
			}
		}
		linebuffer[i++] = c;
	}

	return 0;
}

static int config_file_init(char * config_file)
{
	int		fd;
	int		ret;
	struct config_instance_t * inst;

	fd = open(config_file , O_RDONLY);
	if(fd < 0){
		fprintf(stderr , "read config file %s open fail\n" , config_file);
		return -1;
	}

	while(1){
		inst = config_cft_inst_alloc();
		if (!inst){
			fprintf(stderr , "SUCCESS: parse config file fail\n");
			return 0;
		}
		
		ret = config_readline(fd, inst);
		if(ret < 0){
			fprintf(stderr , "SUCCESS: parse config file fail\n");
			return 0;
		}

		if(ret == 0){
			inst->use = 0;
			break;
		}
	}

	return 0;
}


int config_init(char * config_file)
{
	config_cft_init();

	return config_file_init(config_file);
}
