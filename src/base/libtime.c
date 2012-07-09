
#include <stdio.h>
#include <time.h>

int get_current_seconds(void)
{
	struct tm *local;
	time_t t;

	t = time(NULL);
	return (int)t;
}
