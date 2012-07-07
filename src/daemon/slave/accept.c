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

int do_accept(int listenfd)
{
	int	 		newsockfd;
	int			clilen;
	struct sockaddr_in 	cli_addr;
	int			ret;
	struct pthread_attr_t * pattr;

	for(;;){
		clilen = sizeof(cli_addr);
		newsockfd = accept(listenfd, (struct sockaddr *) &cli_addr, &clilen);
		if(newsockfd < 0){
			fprintf(stderr,"server: accept error\n");
			exit(0);
		}

		pattr = slave_thread_alloc();
		if(pattr == NULL){
			fprintf(stderr , "alloc thread attr fail\n");
			close(newsockfd);
			continue;
		}
		pattr->childfd = newsockfd;
	

		fprintf(stderr,"server: accept one fd %d\n" , newsockfd);

#if (00)
		/* create a new thread to process the incomming request */
		pthread_create(&pattr->tid, NULL , do_child , (void*)pattr);
#endif
		do_child((void*)pattr);

		fprintf(stderr,"server: accept over\n");
		close(newsockfd);
		/* the server is now free to accept another socket request */
	}

	return(0);
}

