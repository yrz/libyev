#ifndef __YEV_H__
#define __YEV_H__

struct yev_loop;

#ifndef ENGINE
#define EPOLL_ENGINE 1
#endif

typedef void yev_cb (struct yev_loop *, int fd, int id, void *data, int mask);

typedef struct yev_ev
{
  int fd;
  int mask;			/* YEV_READ|YEV_WRITE/ONLY_TIME */
  int id;			/* event identifier */
  int timeout;
  yev_cb *cb;
  yev_cb *to_cb;
  int del;
  int add;
  int called;
  void *data;
  struct yev_ev *n;
} yev_ev;

typedef struct yev_loop
{
  int next_event_id;
  yev_ev *f_ev;
  int stop;
#ifdef EPOLL_ENGINE
  int epfd;
#endif
  int engine;
} yev_loop;

#define YEV_OK 1
#define YEV_ERR 0

#define YEV_READ 1
#define YEV_WRITE 2
#define YEV_ONLY_TIME 4

yev_loop *yev_create_loop ();

void yev_del_loop (yev_loop * ev_loop);

yev_ev *yev_get_ev_byid (yev_loop * ev_loop, int id);

void yev_stop (yev_loop * ev_loop);

int yev_create_ev (yev_loop * ev_loop, int fd, int mask, int msecs,
		   yev_cb * cb, yev_cb * to_cb, void *data);

int yev_del_ev (yev_loop * ev_loop, int ev_id);
/* int yev_proc(yev_loop *ev_loop); */
int yev_wait (int fd, int mask, long long msecs);
void yev_main (yev_loop * ev_loop, int ev_stop);
void yev_undel (yev_loop * ev_loop, int id);

#endif
