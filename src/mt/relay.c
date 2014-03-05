#include "../base.h"


static void *step1(void *context){
  void *xmitter=zmq_socket(context, ZMQ_PAIR);
  zmq_connect(xmitter, "inproc://step2");

  debug_log("step 1 ready, signaling step 2\n");
  s_send(xmitter, (char *)"READY", 0, NULL);
  zmq_close(xmitter);

  return NULL;
}


static void *step2(void *context){
  void *receiver=zmq_socket(context, ZMQ_PAIR);
  zmq_bind(receiver, "inproc://step2");

  pthread_t thread;
  pthread_create(&thread, NULL, step1, context);

  char string[256];
  s_recv(receiver, string, 256);
  zmq_close(receiver);

  void *xmitter=zmq_socket(context, ZMQ_PAIR);
  zmq_connect(xmitter, "inproc://step3");
  debug_log("step 2 ready, signaling step 3\n");
  s_send(xmitter, (char *)"READY", 0, NULL);
  zmq_close(xmitter);

  return NULL;
}

int main(){
  void *context=zmq_ctx_new();

  void *receiver=zmq_socket(context, ZMQ_PAIR);
  zmq_bind(receiver, "inproc://step3");

  pthread_t thread;
  pthread_create(&thread, NULL, step2, context);

  char string[256];
  s_recv(receiver, string, 256);
  zmq_close(receiver);

  debug_log("test successful!\n");
  zmq_ctx_destroy(context);

  return 0;
}
