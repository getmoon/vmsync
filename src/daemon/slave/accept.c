#define _REENTRANT
#include "base.h"
#include "priv.h"

__u8            block_id_flag[20000];
__u8		block_id_cnt = 0;

int do_accept(int listenfd)
{
	int	 		newsockfd;
	int			clilen;
	struct sockaddr_in 	cli_addr;
	struct pthread_attr_t * pattr;

	memset(block_id_flag , 0 , 20000);

	for(;;){
		clilen = sizeof(cli_addr);
		newsockfd = accept(listenfd, (struct sockaddr *) &cli_addr, (socklen_t*)&clilen);
		if(newsockfd < 0){
			print_error("server: accept error\n");
			exit(0);
		}

		pattr = slave_thread_alloc();
		if(pattr == NULL){
			print_error("alloc thread attr fail\n");
			close(newsockfd);
			continue;
		}
		pattr->childfd = newsockfd;
	

		print_debug("server: accept one fd %d\n" , newsockfd);

#if (01)
		/* create a new thread to process the incomming request */
		pthread_create(&pattr->tid, NULL , do_child , (void*)pattr);
#else
		do_child((void*)pattr);
#endif

		print_debug("server: accept over\n");
		//close(newsockfd);
		/* the server is now free to accept another socket request */
	}

	return(0);
}

