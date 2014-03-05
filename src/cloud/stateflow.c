#include <czmq.h>
#include "../base.h"


int main(int argc, char *argv[]){
  if (argc<2){
    debug_log("syntax: peering1 me {you}"STR_ELLIPSIS"\n");
    return 0;
  }

  char *self=argv[1];
  debug_log("I: preparing broker at '%s'"STR_ELLIPSIS"\n", self);
  srandom((unsigned)time(NULL));

  zctx_t *ctx=zctx_new();

  void *statebe=zsocket_new(ctx, ZMQ_PUB);
  zsocket_bind(statebe, "ipc://%s-state.ipc", self);

  void *statefe=zsocket_new(ctx, ZMQ_SUB);
  zsocket_set_subscribe(statefe, "");
  int argn;
  for (argn=2; argn<argc; ++argn){
    char *peer=argv[argn];
    debug_log("I: connecting to state backend at '%s'\n", peer);
    zsocket_connect(statefe, "ipc://%s-state.ipc", peer);
  }

  while (1){
    zmq_pollitem_t items[]={{statefe, 0, ZMQ_POLLIN, 0}};
    int rc=zmq_poll(items, 1, 1000*ZMQ_POLL_MSEC);
    if (rc==-1){
      break;
    }

    if (items[0].revents & ZMQ_POLLIN){
      char *peer_name=zstr_recv(statefe);
      char *available=zstr_recv(statefe);
      debug_log("%s - %s workers free\n", peer_name, available);
      free(peer_name);
      free(available);
    }
    else{
      debug_log("send"STR_ELLIPSIS"\n");
      zstr_sendm(statebe, self);
      zstr_send(statebe, "%d", randof(10));
    }
  }

  zctx_destroy(&ctx);

  return 0;
}
