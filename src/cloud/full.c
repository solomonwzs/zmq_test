#include <czmq.h>
#include "../base.h"

#define NBR_CLIENTS 10
#define NBR_WORKERS 5
#define WORKER_READY "\001"

#define localfe_ipc "ipc://%s-localfe.ipc"
#define localbe_ipc "ipc://%s-localbe.ipc"
#define monitor_ipc "ipc://%s-monitor.ipc"
#define cloud_ipc "ipc://%s-cloud.ipc"
#define state_ipc "ipc://%s-state.ipc"

static char *_self;


static void *_client_task(void *args){
  zctx_t *ctx=zctx_new();
  void *client=zsocket_new(ctx, ZMQ_REQ);
  void *monitor=zsocket_new(ctx, ZMQ_PUSH);

  zsocket_connect(client, localfe_ipc, _self);
  zsocket_connect(monitor, monitor_ipc, _self);

  while (true){
    sleep(randof(5));
    int burst=randof(15);
    while (burst--){
      char task_id[5];
      sprintf(task_id, "%04s", randof(0x10000));

      zstr_send(client, task_id);

      zmq_pollitem_t pollset[1]={{client, 0, ZMQ_POLLIN, 0}};
      int rc=zmq_poll(pollset, 1, 10*1000*ZMQ_POLL_MSEC);
      if (rc==-1){
        break;
      }

      if (pollset[0].revents & ZMQ_POLLIN){
        char *reply=zstr_recv(client);
        if (!reply){
          break;
        }

        assert(streq(reply, task_id));
        zstr_send(monitor, "%s", reply);
        free(reply);
      } else{
        zstr_send(monitor, "E: CLIENT EXIT - lost task %s", task_id);
        return NULL;
      }
    }
  }
  zctx_destroy(&ctx);
  return NULL;
}


static void *_worker_task(void *args){
  zctx_t *ctx=zctx_new();
  void *worker=zsocket_new(ctx, ZMQ_REQ);
  zsocket_connect(worker, localbe_ipc, _self);

  zframe_t *frame=zframe_new(WORKER_READY, 1);
  zframe_send(&frame, worker, 0);

  while (true){
    zmsg_t *msg=zmsg_recv(worker);
    if (!msg){
      break;
    }

    sleep(randof(2));
    zmsg_send(&msg, worker);
  }
  zctx_destroy(&ctx);
  return NULL;
}


int main(int argc, char *argv[]){
  if (argc<2){
    debug_log("syntax: peering3 me{you}"STR_ELLIPSIS"\n");
    return 0;
  }
  _self=argv[1];
  debug_log("I: preparing broker at %s"STR_ELLIPSIS"\n", _self);
  srandom((unsigned)time(NULL));

  zctx_t *ctx=zctx_new();

  void *localfe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(localfe, localfe_ipc, _self);

  void *localbe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_bind(localbe, localbe_ipc, _self);

  void *cloudfe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_set_identity(cloudfe, _self);
  zsocket_bind(cloudfe, cloud_ipc, _self);

  void *cloudbe=zsocket_new(ctx, ZMQ_ROUTER);
  zsocket_set_identity(cloudbe, _self);
  int argn;
  for (argn=2; argn<argc; argn++){
    char *peer=argv[argn];
    debug_log("I: connecting to clond frontend at '%s'\n", peer);
    zsocket_connect(cloudbe, cloud_ipc, peer);
  }

  void *statebe=zsocket_new(ctx, ZMQ_SUB);
  zsocket_bind(statebe, state_ipc, _self);

  void *statefe=zsocket_new(ctx, ZMQ_SUB);
  zsocket_set_subscribe(statefe, "");
  for (argn=2; argn<argc; argn++){
    char *peer=argv[argn];
    debug_log("I: connecting to state backend at '%s'\n", peer);
    zsocket_connect(statefe, state_ipc, peer);
  }

  void *monitor=zsocket_new(ctx, ZMQ_PULL);
  zsocket_bind(monitor, monitor_ipc, _self);

  int worker_nbr;
  for (worker_nbr=0; worker_nbr<NBR_WORKERS; worker_nbr++){
    zthread_new(_worker_task, NULL);
  }

  int client_nbr;
  for (client_nbr=0; client_nbr<NBR_CLIENTS; client_nbr++){
    zthread_new(_client_task, NULL);
  }

  int local_capacity=0;
  int cloud_capacity=0;
  zlist_t *workers=zlist_new();

  while (true){
    zmq_pollitem_t primary[]={
      {localbe, 0, ZMQ_POLLIN, 0},
      {cloudbe, 0, ZMQ_POLLIN, 0},
      {statefe, 0, ZMQ_POLLIN, 0},
      {monitor, 0, ZMQ_POLLIN, 0}
    };
  }

  return 0;
}
