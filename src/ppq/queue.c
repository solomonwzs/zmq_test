#include <czmq.h>
#include "../base.h"

#define HEARTBEAT_LIVENESS 3
#define HEARTBEAT_INTERVAL 1000

#define PPP_READY "\001"
#define PPP_HEARTBEAT "\002"

typedef struct{
  zframe_t *identity;
  char *id_string;
  int64_t expiry;
} worker_t;


static worker_t *_worker_new(zframe_t *identity){
  worker_t *self=(worker_t *)zmalloc(sizeof(worker_t));
  self->identity=identity;
  self->id_string=zframe_strhex(identity);
  self->expiry=zclock_time()+HEARTBEAT_INTERVAL*HEARTBEAT_LIVENESS;

  return self;
}


static void _worker_destroy(worker_t **self_p){
  assert(self_p);
  if (*self_p){
    worker_t *self=*self_p;
    zframe_destroy(&self->identity);
    free(self->id_string);
    free(self);
    *self_p=NULL;
  }
}


static void _worker_ready(worker_t *self, zlist_t *workers){
  worker_t *worker=(worker_t *)zlist_first(workers);
  while(worker){
    if (streq(self->id_string, worker->id_string)){
      zlist_remove(workers, worker);
      _worker_destroy(&worker);
      break;
    }
    worker=(worker_t *)zlist_next(workers);
  }
  zlist_append(workers, self);
}


static zframe_t *_worker_next(zlist_t *workers){
  worker_t *worker=zlist_pop(workers);
  assert(worker);
  zframe_t *frame=worker->identity;
  worker->identity=NULL;
  _worker_destroy(&worker);
  return frame;
}


static void _worker_purge(zlist_t *workers){
  worker_t *worker=(worker_t *)zlist_first(workers);
  while (worker){
    if (zclock_time()<worker->expiry){
      break;
    }
    zlist_remove(workers, worker);
    _worker_destroy(&worker);
    worker=(worker_t *)zlist_first(workers);
  }
}


int main(void){
  zctx_t *ctx=zctx_new();
  void *frontend=zsocket_new(ctx, ZMQ_ROUTER);
  void *backend=zsocket_new(ctx, ZMQ_ROUTER);

  zsocket_bind(frontend, "tcp://*:5555");
  zsocket_bind(backend, "tcp://*:5556");

  zlist_t *workers=zlist_new();

  uint64_t heartbeat_at=zclock_time()+HEARTBEAT_INTERVAL;

  while (true){
    zmq_pollitem_t items[]={
      {backend, 0, ZMQ_POLLIN, 0},
      {frontend, 0, ZMQ_POLLIN, 0}
    };

    int rc=zmq_poll(items, zlist_size(workers)?2:1,
                    HEARTBEAT_INTERVAL*ZMQ_POLL_MSEC);
    if (rc==-1){
      break;
    }

    if (items[0].revents & ZMQ_POLLIN){
      zmsg_t *msg=zmsg_recv(backend);
      if (!msg){
        break;
      }

      zframe_t *identity=zmsg_unwrap(msg);
      worker_t *worker=_worker_new(identity);
      _worker_ready(worker, workers);

      if (zmsg_size(msg)==1){
        zframe_t *frame=zmsg_first(msg);
        if (memcmp(zframe_data(frame), PPP_READY, 1)
            && memcmp(zframe_data(frame), PPP_HEARTBEAT, 1)){
          debug_log("E: invalid message from worker\n");
          zmsg_dump(msg);
        }
        zmsg_destroy(&msg);
      } else{
        zmsg_send(&msg, frontend);
      }
    }

    if (items[1].revents & ZMQ_POLLIN){
      zmsg_t *msg=zmsg_recv(frontend);
      if (!msg){
        break;
      }
      zmsg_push(msg, _worker_next(workers));
      zmsg_send(&msg, backend);
    }
  

    if (zclock_time()>=heartbeat_at){
      worker_t *worker=(worker_t *)zlist_first(workers);
      while (worker){
        zframe_send (&worker->identity, backend,
                     ZFRAME_REUSE+ZFRAME_MORE);
        zframe_t *frame=zframe_new(PPP_HEARTBEAT, 1);
        zframe_send(&frame, backend, 0);
        worker=(worker_t *)zlist_next(workers);
      }
      heartbeat_at=zclock_time()+HEARTBEAT_INTERVAL;
    }
    _worker_purge(workers);
  }

  while (zlist_size(workers)){
    worker_t *worker=(worker_t *)zlist_pop(workers);
    _worker_destroy(&worker);
  }
  zlist_destroy(&workers);
  zctx_destroy(&ctx);

  return 0;
}
