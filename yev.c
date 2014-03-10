#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <stdio.h>

#include "yev.h"

yev_loop *yev_create_loop(void)
{
  yev_loop *ev_loop;

  ev_loop = (yev_loop *)calloc(1, sizeof(yev_loop));
  if (!ev_loop)
    return NULL;
  ev_loop->f_ev = NULL;
  ev_loop->next_event_id = 1;
  ev_loop->stop = 0;

#ifdef EPOLL_ENGINE
  ev_loop->epfd = epoll_create(10);
  if ((ev_loop->epfd) < 0)
    return (yev_loop *) YEV_ERR;
#endif
  return ev_loop;
}

void yev_del_loop(yev_loop * ev_loop)
{
#ifdef EPOLL_ENGINE
  close(ev_loop->epfd);
#endif
  free(ev_loop);
}

void yev_stop(yev_loop * ev_loop)
{
  ev_loop->stop = 1;
}

int yev_create_ev(yev_loop * ev_loop, int fd, int mask, int msecs,
		  yev_cb * cb, yev_cb * to_cb, void *data)
{
  yev_ev *e;

  e = (yev_ev *)calloc(1, sizeof(yev_ev));
  if (e == NULL)
    return YEV_ERR;
  int id = ev_loop->next_event_id++;
  e->id = id;
  e->fd = fd;
  e->mask = mask;
  e->timeout = msecs;
  e->cb = cb;
  e->to_cb = to_cb;
  e->data = data;
  e->del = 0;
  e->called = 0;
  e->n = ev_loop->f_ev;
  ev_loop->f_ev = e;
  e->add = 1;

  return id;
}

int __yev_epoll_add_ev(yev_loop * ev_loop, yev_ev * e)
{
  int ret = 0;
  if (e->fd != -1 || !(e->mask & YEV_ONLY_TIME)) {
    struct epoll_event epev;
    epev.events = 0;
    if (e->mask & YEV_READ) {
      epev.events |= EPOLLIN;
    }
    if (e->mask & YEV_WRITE)
      epev.events |= EPOLLOUT;
    epev.data.ptr = e;
    ret = epoll_ctl(ev_loop->epfd, EPOLL_CTL_ADD, e->fd, &epev);
  }
  return ret;
}

yev_ev *yev_get_ev_byid(yev_loop * ev_loop, int id)
{
  yev_ev *e;
  e = ev_loop->f_ev;
  while (e) {
    if (e->id == id)
      return e;
    e = e->n;
  }
  return NULL;
}

void
yev_undel(yev_loop * ev_loop, int id)
{
  yev_ev *e;
  e = yev_get_ev_byid(ev_loop, id);
  e->del = 0;
}


int yev_del_ev(yev_loop * ev_loop, int id)
{
  yev_ev *e, *prev = NULL;
  int fd;

  e = ev_loop->f_ev;

  while (e) {
    if (e->id == id) {
      fd = e->fd;
      if (prev == NULL)
	ev_loop->f_ev = e->n;
      else
	prev->n = e->n;
      free(e);

#ifdef EPOLL_ENGINE
	if (fd != -1) {
	  struct epoll_event event;
	  int ret;
	  ret = epoll_ctl(ev_loop->epfd, EPOLL_CTL_DEL, fd, &event);
	  if (ret)
	    return YEV_ERR;
	}
#endif

      return YEV_OK;
    }
    prev = e;
    e = e->n;
  }
  return YEV_OK;
}


static 
yev_ev *_yev_nearest(yev_loop * ev_loop)
{
  yev_ev *e = ev_loop->f_ev;
  yev_ev *nearest = NULL;

  while (e) {
    if (!nearest || (e->timeout < nearest->timeout && e->timeout > -1))
      nearest = e;
    e = e->n;
  }
  return nearest;
}

int _ms_diff(struct timeval *tv)
{
  struct timeval o, d;
	
  o = *tv;
  gettimeofday(tv, NULL);
  timersub(tv, &o, &d);

  return (d.tv_sec * 1000 + d.tv_usec / 1000);
}

static
void _yev_update_ev(yev_loop * ev_loop, int expired_ms)
{
  yev_ev *e, *n;

  e = ev_loop->f_ev;
  while (e) {
    n = e->n;
    if (e->del) {
      yev_del_ev(ev_loop, e->id);
    }
    if (!e->called && e->timeout > 0 && expired_ms > 0) {
      e->timeout = e->timeout > expired_ms ? e->timeout - expired_ms : 0;
    }
    e->called = 0;
    e = n;
  }
  e = ev_loop->f_ev;
  while (e) {
    if (e->add) {
#ifdef EPOLL_ENGINE
	__yev_epoll_add_ev(ev_loop, e);
#endif
      e->add = 0;
    }
    e = e->n;
  }
}

static
int _yev_only_time_loop(yev_loop * ev_loop)
{
  yev_ev *e;

  e = ev_loop->f_ev;
  while (e) {
    if (e->mask & YEV_READ || e->mask & YEV_WRITE)
      return 0;
    e = e->n;
  }
  return 1;
}

// static
// void __dump_event(yev_ev * e)
// {
//    printf("event %p id <%d>\n", e, e->id);
//    printf("\t next <%p>\n", e->n);
//    printf("\t fd <%d>\n", e->fd);
//    printf("\t mask <%d>\n", e->mask);
// }
// 
// static
// void __dump_loop(yev_loop * ev_loop)
// {
//   struct yev_ev *e;
// 
//   e = ev_loop->f_ev;
// 
//   while (e) {
//     __dump_event(e);
//     e = e->n;
//   }
// }

static
int _yev_epoll_proc(yev_loop * ev_loop)
{
  int ret = 0;

  yev_ev *n, *e;
  struct epoll_event *events;
  int nr_events, i;
  const int max_events = 1;
  int expired_ms = 0;
  struct timeval tv;


  n = _yev_nearest(ev_loop);
  if (!n) {
    return YEV_ERR;
  }

  gettimeofday(&tv, NULL);

  _yev_update_ev(ev_loop, 0);
  events = calloc(max_events, sizeof(struct epoll_event));
  do {
    if (!_yev_only_time_loop(ev_loop)) {
//      printf("timeout %d\n", n->timeout);
      nr_events = epoll_wait(ev_loop->epfd, events, max_events, n->timeout);
    } else {
      nr_events = 0;
      usleep(n->timeout * 1000);
    }

    if (nr_events < 0)
      return YEV_ERR;

    if (nr_events == 0) {
      n->del = 1;
      n->called = 1;
      n->to_cb(ev_loop, n->fd, n->id, n->data, n->mask);
    } else {
      for (i = 0; i < nr_events; i++) {
	e = events[i].data.ptr;
	e->del = 1;
	e->called = 1;
	if(e->cb) e->cb(ev_loop, e->fd, e->id, e->data, e->mask);
      }
    }
    expired_ms = _ms_diff(&tv);
//    printf("expired %d ms\n", expired_ms);
    _yev_update_ev(ev_loop, expired_ms);
    //__dump_loop(ev_loop);
    n = _yev_nearest(ev_loop);
  } while (ev_loop->f_ev);

  free(events);
  return ret;
}

static
int _yev_proc(yev_loop * ev_loop)
{
#ifdef EPOLL_ENGINE
  return _yev_epoll_proc(ev_loop);
#endif
  return -1;
}

void yev_main(yev_loop *ev_loop, int ev_stop)
{
  ev_loop->stop = 0;
  while (!ev_loop->stop) 
	  if(_yev_proc(ev_loop) == YEV_ERR && ev_stop==1)
		  ev_loop->stop = 1;
}
