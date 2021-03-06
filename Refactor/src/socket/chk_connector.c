#define _CORE_
#include <assert.h>
#include "socket/chk_connector.h"
#include "event/chk_event_loop.h"
#include "socket/chk_socket_helper.h"
#include "util/chk_log.h"
#include "util/chk_error.h"

#ifndef  cast
# define  cast(T,P) ((T)(P))
#endif

typedef struct chk_connector chk_connector;

struct chk_connector {
    _chk_handle; 
    void      *ud;
    connect_cb cb;
    uint32_t   timeout;
    chk_timer *t;    
};

int32_t connect_timeout(uint64_t tick,void *ud) {
	chk_connector *c = cast(chk_connector*,ud);
	c->cb(-1,ETIMEDOUT,c->ud);
	close(c->fd);
	free(c);
	return -1;
}

static int32_t loop_add(chk_event_loop *e,chk_handle *h,chk_event_callback cb) {
	int32_t ret;
	chk_connector *c = cast(chk_connector*,h);
	assert(e && h && cb);
	ret = chk_events_add(e,h,CHK_EVENT_READ) || chk_events_add(e,h,CHK_EVENT_WRITE);
	if(ret == 0) {
		c->cb = cast(connect_cb,cb);
		if(c->timeout) {
			c->t = chk_loop_addtimer(e,c->timeout,connect_timeout,c);
		}			
	}
	return ret;
}

static void _process_connect(chk_connector *c) {
	int32_t err = 0;
	int32_t fd = -1;
	socklen_t len = sizeof(err);
	if(c->t) {
		chk_timer_unregister(c->t);
		c->t = NULL;
	}
	do {
		if(getsockopt(c->fd, SOL_SOCKET, SO_ERROR, &err, &len) == -1){
			c->cb(-1,err,c->ud);
		    break;
		}
		if(err) {
		    errno = err;
		    c->cb(-1,err,c->ud);    
		    break;
		}
		//success
		fd = c->fd;
	}while(0);
	chk_events_remove(cast(chk_handle*,c));    
	if(fd != -1) c->cb(fd,0,c->ud);
	else close(c->fd);	
}

static void process_connect(chk_handle *h,int32_t events) {
	chk_connector *c = cast(chk_connector*,h);
	if(events == CHK_EVENT_ECLOSE){
		c->cb(-1,CHK_ELOOPCLOSE,c->ud);
		close(c->fd);
	}else _process_connect(c);
	free(h);
}

chk_connector *chk_connector_new(int32_t fd,void *ud,uint32_t timeout) {
	chk_connector *c = calloc(1,sizeof(*c));
	c->fd = fd;
	c->on_events  = process_connect;
	c->handle_add = loop_add;
	c->timeout = timeout;
	c->ud = ud;
	easy_close_on_exec(fd);
	return c;
}

int32_t chk_connect(int32_t fd,chk_sockaddr *server,chk_sockaddr *local,
					chk_event_loop *e,connect_cb cb,void*ud,uint32_t timeout) {
	chk_connector *c;
	int32_t        ret;
	if(!cb) {
		easy_noblock(fd,0);
		ret = easy_connect(fd,server,local);
	} else {
		easy_noblock(fd,1);
		if(0 == (ret = easy_connect(fd,server,local)))
			cb(fd,0,ud);
		else if(ret == -EINPROGRESS){
			c   = chk_connector_new(fd,ud,timeout);
			chk_loop_add_handle(e,cast(chk_handle*,c),cast(chk_event_callback,cb));
			ret = 0;			
		}else {
			close(fd);
			ret = -ret;
		}	
	}
	return ret;
}




