#include "../base.h"


int main(){
  void *context=zmq_ctx_new();
  void *requester=zmq_socket(context, ZMQ_REQ);
  zmq_connect(requester, "tcp://localhost:5559");

  int request_nbr;
  char string[256];
  for (request_nbr=0; request_nbr<10; ++request_nbr){
    s_send(requester, (char *)"Hello", 0, NULL);
    s_recv(requester, string, 256);
    debug_log("Received response: [%s]\n", string);
  }

  zmq_close(requester);
  zmq_ctx_destroy(context);
  return 0;
}
