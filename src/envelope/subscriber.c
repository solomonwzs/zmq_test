#include "../base.h"

#define FILETER_KEY "B"
#define FILETER_ADDR "address3"


int main(){
  void *context=zmq_ctx_new();
  void *subscriber=zmq_socket(context, ZMQ_SUB);
  int rc=zmq_connect(subscriber, "tcp://localhost:5563");
  assert(rc==0);

  zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, FILETER_KEY, strlen(FILETER_KEY));
  //zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, FILETER_ADDR, strlen(FILETER_ADDR));

  char key[256];
  char address[256];
  char content[256];
  while(1){
    s_recv(subscriber, key, 256);
    s_recv(subscriber, address, 256);
    s_recv(subscriber, content, 256);

    debug_log("[%s] <\"%s\"> %s\n", key, address, content);
  }

  zmq_close(subscriber);
  zmq_ctx_destroy(context);

  return 0;
}
