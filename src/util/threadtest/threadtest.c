

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/uio.h>
#include <unistd.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <pthread.h>

#define RTE_DEFINE_PER_LCORE(type, name)			\
	__thread __typeof__(type) per_lcore_##name

#define RTE_DECLARE_PER_LCORE(type, name)			\
	extern __thread __typeof__(type) per_lcore_##name

#define RTE_PER_LCORE(name) (per_lcore_##name)

typedef struct __test_t{
	int	a;
	int	b;
	int	c;
}test_t;


RTE_DEFINE_PER_LCORE(test_t , tt);

int main(int argc , char ** argv)
{



	return 0;
}





