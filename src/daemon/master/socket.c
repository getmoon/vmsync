#include "base.h"
#include "msg.h"


int remote_connect(char * serverip , unsigned short serverport)
{
	int sock;                        /* Socket descriptor */
	struct sockaddr_in echoServAddr; /* Echo server address */
       
	
	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
		print_info("socket() failed");
		return -1;
	}

	/* Construct the server address structure */
	memset(&echoServAddr, 0, sizeof(echoServAddr));     /* Zero out structure */
	echoServAddr.sin_family      = AF_INET;             /* Internet address family */
	echoServAddr.sin_addr.s_addr = inet_addr(serverip);   /* Server IP address */
	echoServAddr.sin_port        = htons(serverport); /* Server port */

	/* Establish the connection to the echo server */
	if (connect(sock, (struct sockaddr *) &echoServAddr, sizeof(echoServAddr)) < 0){
		print_error("connect() failed");
		return -1;
	}

	return sock;
}

