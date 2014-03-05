#include "../base.h"


int main(){
  void *context=zmq_ctx_new();
  void *responder=zmq_socket(context, ZMQ_REP);
  zmq_connect(responder, "tcp://localhost:5560");

  char string[256];
  while(1){
    s_recv(responder, string, 256);
    debug_log("Received request: [%s]\n", string);

    sleep(1);

    s_send(responder, (char *)"World!", 0, NULL);
  }

  zmq_close(responder);
  zmq_ctx_destroy(context);
  return 0;
}
