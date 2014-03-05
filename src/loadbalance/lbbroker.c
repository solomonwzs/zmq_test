#include "../base.h"

#define NBR_CLIENTS 10
#define NBR_WORKERS 3

#define FRONTEND "ipc://frontend.ipc"
#define BACKEND "ipc://backend.ipc"

#define DEQUEUE(_q_) memmove(&(_q_)[0], &(_q_)[1], sizeof(_q_)-sizeof(_q_[0]))

#define _recv_empty(_s_, _e_) do{\
  s_recv(_s_, _e_, 2);\
  assert(_e_[0]==0);\
} while(0)

#define _scan_queue(_q_) do{\
  char **__i;\
  for (__i=_q_; *__i; ++__i){\
    printf("%s ", *__i);\
  }\
  printf("\n");\
} while(0)


static void _strcat(char *dest, unsigned count, ...){
  va_list ap;
  unsigned i;
  char *src;

  dest[0]='\0';
  va_start(ap, count);
  for (i=0; i<count; i++){
    src=va_arg(ap, char *);
    strcat(dest, src);
  }
  va_end(ap);
}


static void *_client_task(void *args){
  void *context=zmq_ctx_new();
  void *client=zmq_socket(context, ZMQ_REQ);
  s_set_id(client);
  zmq_connect(client, FRONTEND);

  char client_id[16];
  size_t size=16;
  zmq_getsockopt(client, ZMQ_IDENTITY, client_id, &size);

  char msg[256];
  _strcat(msg, 3, "c[", client_id, "]:hello");

  s_send(client, msg, 0, NULL);
  //debug_log("c[%s] send msg\n", client_id);

  char reply[256];
  s_recv(client, reply, 256);
  debug_log("c[%s] receive: %s\n", client_id, reply);

  zmq_close(client);
  zmq_ctx_destroy(context);
  return NULL;
}


static void *_worker_task(void *args){
  void *context=zmq_ctx_new();
  void *worker=zmq_socket(context, ZMQ_REQ);
  s_set_id(worker);
  zmq_connect(worker, BACKEND);

  char worker_id[16];
  size_t size=16;
  zmq_getsockopt(worker, ZMQ_IDENTITY, worker_id, &size);

  char msg[256];
  _strcat(msg, 3, "w[", worker_id, "]:ready");
  //debug_log("%s\n", msg);

  s_send(worker, msg, 0, NULL);

  char id[256];
  char empty[2];
  char request[256];
  while (1){
    s_recv(worker, id, 256);
    //debug_log("w[%s] receive id: %s\n", worker_id, id);
    _recv_empty(worker, empty);

    s_recv(worker, request, 256);
    //debug_log("w[%s] receive: %s\n", worker_id, request);

    _strcat(msg, 3, "w[", worker_id, "]:ok");

    //debug_log("-->%s\n", id);
    s_sendlists(worker, 3, id, 0, "", 0, msg, 0);
    //s_sendmore(worker, id, 0, NULL);
    //s_sendmore(worker, "", 0, NULL);
    //s_send(worker, msg, 0, NULL);
  }

  zmq_close(worker);
  zmq_ctx_destroy(context);

  return NULL;
}


int main(){
  void *context=zmq_ctx_new();
  void *frontend=zmq_socket(context, ZMQ_ROUTER);
  void *backend=zmq_socket(context, ZMQ_ROUTER);
  zmq_bind(frontend, FRONTEND);
  zmq_bind(backend, BACKEND);

  int client_nbr;
  for (client_nbr=0; client_nbr<NBR_CLIENTS; ++client_nbr){
    pthread_t client;
    pthread_create(&client, NULL, _client_task, NULL);
  }
  int worker_nbr;
  for (worker_nbr=0; worker_nbr<NBR_WORKERS; ++worker_nbr){
    pthread_t worker;
    pthread_create(&worker, NULL, _worker_task, NULL);
  }

  int available_workers=0, i;
  char *worker_queue[10];
  for (i=0; i<10; ++i){
    worker_queue[i]=NULL;
  }
  char empty[2];
  char worker_id[256];
  char client_id[256];
  char msg[256];

  while (1){
    zmq_pollitem_t items[]={
      {backend, 0, ZMQ_POLLIN, 0},
      {frontend, 0, ZMQ_POLLIN, 0}
    };
    int rc=zmq_poll(items, available_workers?2:1, -1);
    if (rc==-1){
      break;
    }

    if (items[0].revents & ZMQ_POLLIN){
      s_recv(backend, worker_id, 256);
      assert(available_workers<NBR_WORKERS);

      char *id=malloc(sizeof(char)*strlen(worker_id));
      strcpy(id, worker_id);
      worker_queue[available_workers++]=id;

      _recv_empty(backend, empty);

      s_recv(backend, client_id, 256);
      if (client_id[0]!='w'){
      //if (strcmp(client_id, "ready")!=0){
        _recv_empty(backend, empty);
        s_recv(backend, msg, 256);
        s_sendlists(frontend, 3, client_id, 0, "", 0, msg, 0);

        if (--client_nbr==0){
          break;
        }
      }
    }

    if (items[1].revents & ZMQ_POLLIN){
      s_recv(frontend, client_id, 256);
      _recv_empty(frontend, empty);
      s_recv(frontend, msg, 256);

      //debug_log(":%s >>> %s\n", client_id, worker_queue[0]);
      s_sendlists(backend, 5,
                  worker_queue[0], 0, "", 0, client_id, 0, "", 0, msg, 0);

      free(worker_queue[0]);
      DEQUEUE(worker_queue);
      available_workers--;
    }
  }

  zmq_close(backend);
  zmq_close(frontend);
  zmq_ctx_destroy(context);
  return 0;
}
