#include "base.h"


static int inet_pton4 (const char * src, unsigned char * dst)
{
  int saw_digit, octets, ch;
  unsigned char tmp[4], *tp;

  saw_digit = 0;
  octets = 0;
  *(tp = tmp) = 0;
  while ((ch = *src++) != '\0')
    {

      if (ch >= '0' && ch <= '9')
	{
	  unsigned new = *tp * 10 + (ch - '0');

	  if (saw_digit && *tp == 0)
	    return (0);
	  if (new > 255)
	    return (0);
	  *tp = new;
	  if (!saw_digit)
	    {
	      if (++octets > 4)
		return (0);
	      saw_digit = 1;
	    }
	}
      else if (ch == '.' && saw_digit)
	{
	  if (octets == 4)
	    return (0);
	  *++tp = 0;
	  saw_digit = 0;
	}
      else
	return (0);
    }
  if (octets < 4)
    return (0);
  memcpy (dst, tmp, 4);
  return (1);
}

int parse_ip(char *arg, uint32_t *ip)
{
	int ret;
	struct in_addr addr;

	if(arg == NULL || ip == NULL) {
		ret = -1;
		goto out;
	}

        ret = inet_pton4(arg, (unsigned char* )&addr);
	if(ret == 0) {
		ret = -1;
		goto out;
	}
	*ip = ntohl(addr.s_addr);

	ret = 0;
out:
	return ret;
}

int parse_u32(char *arg, uint32_t *num)
{
	long long ret;

	if(arg == NULL || num == NULL) {
		ret = -1;
		goto out;
	}

	ret = is_number(arg);
	if(ret == FALSE) {
		ret = -1;
		goto out;
	}

	ret = strtoll(arg, NULL, 0);
	if(ret < 0 || ret > 0xffffffff) {
		ret = -1;
		goto out;
	}

	*num = ret;
	ret = 0;
out:
	return ret;
}


