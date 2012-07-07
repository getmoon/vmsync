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

#include "priv.h"

#define TCP_PORT   	6500

int slave_socket_init(void)
{
	int 			sockfd;
	struct sockaddr_in	serv_addr;
	int			optval = -1;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		fprintf(stderr,"server: can't open stream socket\n");
		exit(0);
	}

	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int))<0){
		fprintf(stderr , "setsockopt socket REUSE fail\n");
		exit(0);
	}
 
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(TCP_PORT);
	
	if(bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		fprintf(stderr,"server: can't bind local address\n");
		exit(0);
	}

	listen(sockfd, 100);
	return sockfd;
}

