#include <stdio.h>
#include <time.h>

int get_current_seconds(void)
{
	time_t t;

	t = time(NULL);
	return (int)t;
}
