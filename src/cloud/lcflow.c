#include <czmq.h>
#include "../base.h"


#define NBR_CLIENTS 10
#define NBR_WORKERS 3
#define WORKER_READY "\001"


static char *_self;


static void *_client_task(void *args){
  zctx_t *ctx=zctx_new();
  void *client=zsocket_new(ctx, ZMQ_REP);
  zsocket_connect(client, "ipc://%s-localfe.ipc", _self);

  while (1){
    zstr_send(client, "HELLO");
    char *reply=zstr_recv(client);
    if (!reply){
      break;
    }

    debug_log("Client: %s\n", reply);
    free(reply);
    sleep(1);
  }
  zctx_destroy(&ctx);
  return NULL;
}


static void *_worker_task(void *args){
  zctx_t *ctx=zctx_new();
  void *worker=zsocket_new(ctx, ZMQ_REP);
  zsocket_connect(worker, "ipc://%s-localbe.ipc", _self);

  zframe_t *frame=zframe_new(WORKER_READY, 1);
  zframe_send(&frame, worker, 0);

  while (1){
    zmsg_t *msg=zmsg_recv(worker);
    if (!msg){
      break;
    }

    zframe_print(zmsg_last(msg), "Worker: ");
    zframe_reset(zmsg_last(msg), "OK", 2);
    zmsg_send(&msg, worker);
  }
  zctx_destroy(&ctx);
  return NULL;
}

int main(int argc, char *argv[]){
  if (argc<2){
    debug_log("syntax: peering2 me {you}"STR_ELLIPSIS"\n");
    return 0;
  }
  _self=argv[1];
  debug_log("I: preparing broker at '%s'"STR_ELLIPSIS"\n", _self);
  srandom((unsigned)time(NULL));

  zctx_t *ctx=zctx_new();

  void *cloudfe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_set_identity(cloudfe, _self);
  zsocket_bind(cloudfe, "ipc://%s-cloud.ipc", _self);

  void *cloudbe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_set_identity(cloudbe, _self);
  int argn;
  for (argn=2; argn<argc; ++argn){
    char *peer=argv[argn];
    debug_log("I: connecting to cloud frontend at '%s'\n", peer);
    zsocket_connect(cloudbe, "ipc://%s-cloud.ipc", peer);
  }

  void *localfe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(localfe, "ipc://%s-localfe.ipc", _self);
  void *localbe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(localbe, "ipc://%s-localbe.ipc", _self);

  debug_log("Press Enter when all brokers are started: ");
  getchar();

  int worker_nbr;
  for (worker_nbr=0; worker_nbr<NBR_WORKERS; ++worker_nbr){
    zthread_new(_worker_task, NULL);
  }

  int client_nbr;
  for (client_nbr=0; client_nbr<NBR_CLIENTS; ++client_nbr){
    zthread_new(_client_task, NULL);
  }

  int capacity=0;
  zlist_t *workers=zlist_new();

  while (1){
    zmq_pollitem_t backends[]={
      {localbe, 0, ZMQ_POLLIN, 0},
      {cloudbe, 0, ZMQ_POLLIN, 0}
    };

    int rc=zmq_poll(backends, 2, capacity?1000*ZMQ_POLL_MSEC:-1);
    if (rc==-1){
      break;
    }

    zmsg_t *msg=NULL;
    if (backends[0].revents & POLL_IN){
      msg=zmsg_recv(localbe);
      if (!msg){
        break;
      }

      zframe_t *id=zmsg_unwrap(msg);
      zlist_append(workers, id);
      capacity++;

      zframe_t *frame=zmsg_first(msg);
      if (memcmp(zframe_data(frame), WORKER_READY, 1)==0){
        zmsg_destroy(&msg);
      }
    } else if (backends[1].revents & ZMQ_POLLIN){
      msg=zmsg_recv(cloudbe);
      if (!msg){
        break;
      }
      zframe_t *id=zmsg_unwrap(msg);
      zframe_destroy(&id);
    }

    for (argn=2; msg && argn<argc; ++argn){
      char *data=(char *)zframe_data(zmsg_first(msg));
      size_t size=zframe_size(zmsg_first(msg));
      if (size==strlen(argv[argn]) && memcmp(data, argv[argn], size)==0){
        zmsg_send(&msg, cloudfe);
      }
    }

    if (msg){
      zmsg_send(&msg, localfe);
    }

    while (capacity){
      zmq_pollitem_t frontends[]={
        {localfe, 0, ZMQ_POLLIN, 0},
        {cloudfe, 0, ZMQ_POLLIN, 0}
      };
      rc=zmq_poll(frontends, 2, 0);
      assert(rc>=0);
      int reroutable=0;

      if (frontends[1].revents & ZMQ_POLLIN){
        msg=zmsg_recv(cloudfe);
        reroutable=0;
      } else if (frontends[0].revents & ZMQ_POLLIN){
        msg=zmsg_recv(localfe);
        reroutable=1;
      } else{
        break;
      }

      if (reroutable && argc>2 && randof(5)==0){
        int peer=randof(argc-2)+2;
        zmsg_pushmem(msg, argv[peer], strlen(argv[peer]));
        zmsg_send(&msg, cloudbe);
      } else{
        zframe_t *frame=(zframe_t *)zlist_pop(workers);
        zmsg_wrap(msg, frame);
        zmsg_send(&msg, localbe);
        --capacity;
      }
    }
  }

  while (zlist_size(workers)){
    zframe_t *frame=(zframe_t *)zlist_pop(workers);
    zframe_destroy(&frame);
  }
  zlist_destroy(&workers);
  zctx_destroy(&ctx);

  return EXIT_SUCCESS;
}
