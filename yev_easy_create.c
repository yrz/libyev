#ifdef __YEV_NO_MACRO__

#include <stdlib.h>
#include "yev.h"
#include "yev_easy_create.h"

int yev_rwio(yev_loop * ev_loop, int fd, yev_cb * cb)
{
  return yev_create_ev(ev_loop, fd, YEV_READ|YEV_WRITE, -1, cb, NULL, NULL);
}

int yev_rio(yev_loop * ev_loop, int fd, yev_cb * cb)
{
  return yev_create_ev(ev_loop, fd, YEV_READ, -1, cb, NULL, NULL);
}

int yev_wio(yev_loop * ev_loop, int fd, yev_cb * cb)
{
  return yev_create_ev(ev_loop, fd, YEV_WRITE, -1, cb, NULL, NULL);
}

int yev_to(yev_loop * ev_loop, int timeout, yev_cb * cb)
{
  return yev_create_ev(ev_loop, -1, YEV_ONLY_TIME, timeout, NULL, cb, NULL);
}

#endif /* __YEV_NO_MACRO__ */
