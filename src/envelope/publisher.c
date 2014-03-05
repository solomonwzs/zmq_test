#include "../base.h"


int main(){
  void *context=zmq_ctx_new();
  void *publisher=zmq_socket(context, ZMQ_PUB);
  int rc=zmq_bind(publisher, "tcp://*:5563");
  assert(rc==0);

  while(1){
    debug_log("send msg\n");
    s_sendmore(publisher, "A", 0, NULL);
    s_sendmore(publisher, "address1", 0, NULL);
    s_send(publisher, "msg1", 0, NULL);

    s_sendmore(publisher, "B", 0, NULL);
    s_sendmore(publisher, "address2", 0, NULL);
    s_send(publisher, "msg2", 0, NULL);

    s_sendmore(publisher, "B", 0, NULL);
    s_sendmore(publisher, "address3", 0, NULL);
    s_send(publisher, "msg3", 0, NULL);

    sleep(1);
  }

  zmq_close(publisher);
  zmq_ctx_destroy(context);

  return 0;
}
