#ifndef __NETDEF_H__
#define __NETDEF_H__

#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "algorithm/Queue.h"

#define MAXFD 4096
#define BUFFSIZE 1024

typedef struct sockaddr SA;

struct NewConnInfo
{
	int fd;
	struct sockaddr_in addr;
	uint32_t addrLen;

	NewConnInfo()
	{
		fd = -1;
		memset(&addr, 0, sizeof(addr));
		addrLen = 0;
	}
};

#endif
