#include "base.h"

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





