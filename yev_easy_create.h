#ifndef __YEV_EASY_H__
#define __YEV_EASY_H__

#include "yev.h"

#ifdef __YEV_NO_MACRO__

int yev_rio (yev_loop * ev_loop, int fd, yev_cb * cb);
int yev_wio (yev_loop * ev_loop, int fd, yev_cb * cb);
int yev_rwio (yev_loop * ev_loop, int fd, yev_cb * cb);
int yev_to (yev_loop * ev_loop, int timeout, yev_cb * cb);

#else /* __YEV_NO_MACRO__ */

#define  yev_rwio(loop, fd, cb) \
         yev_create_ev(loop, fd, YEV_READ|YEV_WRITE, -1, cb, NULL, NULL);
#define  yev_rio(loop, fd, cb) \
         yev_create_ev(loop, fd, YEV_READ, -1, cb, NULL, NULL); 
#define  yev_wio(loop, fd, cb) \
         yev_create_ev(loop, fd, YEV_WRITE, -1, cb, NULL, NULL); 
#define  yev_to(loop, timeout, cb) \
         yev_create_ev(loop, -1, YEV_ONLY_TIME, timeout, NULL, cb, NULL); 

#define  yev_rwiod(loop, fd, cb, data) \
         yev_create_ev(loop, fd, YEV_READ|YEV_WRITE, -1, cb, NULL, data);
#define  yev_riod(loop, fd, cb, data) \
         yev_create_ev(loop, fd, YEV_READ, -1, cb, NULL, data); 
#define  yev_wiod(loop, fd, cb, data) \
         yev_create_ev(loop, fd, YEV_WRITE, -1, cb, NULL, data); 
#define  yev_tod(loop, timeout, cb, data) \
         yev_create_ev(loop, -1, YEV_ONLY_TIME, timeout, NULL, cb, data); 

#endif /* __YEV_NO_MACRO__ */
#endif /* __YEV_EASY_H__ */
