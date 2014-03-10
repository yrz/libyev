#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "yev.h"
#include "yev_easy_create.h"

void _cb(struct yev_loop *ev_loop, int fd, int id, void *data, int mask)
{
  yev_ev *e;
  struct timeval now, *tv;
  long long expired_msecs;
  
  gettimeofday(&now, NULL);
  e = yev_get_ev_byid(ev_loop, id);
  e->del = 0;
  e->timeout -= 10;
  if(e->timeout < 1) e->del = 1;
  tv = (struct timeval *)data;

  expired_msecs = (now.tv_sec - tv->tv_sec)*1000;
  expired_msecs += (now.tv_usec - tv->tv_usec)/1000;
  printf("called event with id[%d] after %lli milliseconds", id, expired_msecs);
	if(e->timeout > 0) 
	{
	  printf(", next expected at %lld", expired_msecs+e->timeout);
	}
	puts("");
}

int main(int argc, char **argv)
{
  yev_loop *ev_loop;
  struct timeval *tv;
  tv = malloc(sizeof(*tv));
  gettimeofday(tv, NULL);
  int ret;
  
  ev_loop = yev_create_loop(EPOLL_ENGINE);
  ret = yev_tod(ev_loop, 300, _cb, tv);
  printf("id[%d]\n", ret);
	
  yev_main(ev_loop, 1);
  free(tv);
  free(ev_loop);
  return 0;
}
