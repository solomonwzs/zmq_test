#include "../base.h"


int main(){
  void *context=zmq_ctx_new();
  void *sink=zmq_socket(context, ZMQ_ROUTER);
  zmq_bind(sink, "inproc://example");

  void *anonymous=zmq_socket(context, ZMQ_REQ);
  zmq_connect(anonymous, "inproc://example");
  s_send(anonymous, "ROUTER uses a generated UUID", 0, NULL);
  s_dump(sink);

  void *identified=zmq_socket(context, ZMQ_REQ);
  zmq_setsockopt(identified, ZMQ_IDENTITY, "PEER2", 5);
  zmq_connect(identified, "inproc://example");
  s_send(identified, "ROUTER socker uses REQ's socker identity", 0, NULL);
  s_dump(sink);

  zmq_close(identified);
  zmq_close(anonymous);
  zmq_close(sink);
  zmq_ctx_destroy(context);
  return 0;
}
