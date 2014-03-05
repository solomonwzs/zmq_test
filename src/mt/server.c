#include "../base.h"


struct ctx{
  int id;
  void *context;
};


static void *worker_routine(void *c){
  struct ctx *p=c;
  void *context=p->context;
  int id=p->id;

  free(p);

  void *receiver=zmq_socket(context, ZMQ_REP);
  zmq_connect(receiver, "inproc://workers");

  char string[256];
  while(1){
    s_recv(receiver, string, 256);
    debug_log("id: %d, received request: [%s]\n", id, string);

    sleep(1);
    s_send(receiver, (char *)"World", 0, NULL);
  }
  zmq_close(receiver);

  return NULL;
}


int main(){
  void *context=zmq_ctx_new();

  void *clients=zmq_socket(context, ZMQ_ROUTER);
  zmq_bind(clients, "tcp://*:5555");

  void *workers=zmq_socket(context, ZMQ_DEALER);
  zmq_bind(workers, "inproc://workers");

  int thread_nbr;
  for (thread_nbr=0; thread_nbr<5; ++thread_nbr){
    struct ctx *c=malloc(sizeof(struct ctx));
    c->id=thread_nbr;
    c->context=context;

    pthread_t worker;
    pthread_create(&worker, NULL, worker_routine, c);
  }
  zmq_proxy(clients, workers, NULL);

  zmq_close(workers);
  zmq_close(clients);
  zmq_ctx_destroy(context);

  return 0;
}
