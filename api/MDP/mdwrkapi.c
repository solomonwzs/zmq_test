#include "mdwrkapi.h"

#define HEARTBEAT_LIVENESS 3


static void _mdwrk_send_to_broken(mdwrk_t *self, char *command, char *option,
                                  zmsg_t *msg){
  msg=msg?zmsg_dup(msg):zmsg_new();

  if (option){
    zmsg_pushstr(msg, option);
  }
  zmsg_pushstr(msg, command);
  zmsg_pushstr(msg, MDPW_WORKER);
  zmsg_pushstr(msg, "");

  if (self->verbose){
    debug_log("I: sending %s to broker\n", mdps_commands[(int) *command]);
    zmsg_dump(msg);
  }
  zmsg_send(&msg, self->worker);
}


static void _mdwrk_connect_to_broken(mdwrk_t *self){
  if (self->worker){
    zsocket_destroy(self->ctx, self->worker);
  }
  self->worker=zsocket_new(self->ctx, ZMQ_DEALER);
  zmq_connect(self->worker, self->broker);
  if (self->verbose){
    debug_log("I: connecting to broker at %s"STR_ELLIPSIS"\n", self->broker);
  }

  _mdwrk_send_to_broken(self, MDPW_READY, self->service, NULL);

  self->liveness=HEARTBEAT_LIVENESS;
  self->heartbeat_at=zclock_time()+self->heartbeat;
}


mdwrk_t *mdwrk_new(char *broker, char *service, int verbose){
  assert(broker && service);

  mdwrk_t *self=(mdwrk_t *)zmalloc(sizeof(mdwrk_t));
  self->ctx=zctx_new();
  self->broker=strdup(broker);
  self->service=strdup(service);
  self->verbose=verbose;
  self->heartbeat=2500;
  self->reconnect=2500;

  _mdwrk_connect_to_broken(self);
  return self;
}


void mdwrk_destroy(mdwrk_t **self_p){
  assert(self_p);
  if (*self_p){
    mdwrk_t *self=*self_p;
    zctx_destroy(&self->ctx);
    free(self->broker);
    free(self->service);
    free(self);
    *self_p=NULL;
  }
}


zmsg_t *mdwrk_recv(mdwrk_t *self, zmsg_t **reply_p){
  assert(reply_p);
  zmsg_t *reply=*reply_p;
  assert(reply || !self->expect_reply);
  if (reply){
    assert(self->reply_to);
    zmsg_wrap(reply, self->reply_to);
    _mdwrk_send_to_broken(self, MDPW_REPLY, NULL, reply);
    zmsg_destroy(reply_p);
  }
  self->expect_reply=1;

  while (true){
    zmq_pollitem_t items[]={{self->worker, 0, ZMQ_POLLIN, 0}};
    int rc=zmq_poll(items, 1, self->heartbeat*ZMQ_POLL_MSEC);
    if (rc==-1){
      break;
    }

    if (items[0].revents & ZMQ_POLLIN){
      zmsg_t *msg=zmsg_recv(self->worker);
      //...
    }
  }
}
