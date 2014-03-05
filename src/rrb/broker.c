#include "../base.h"


#define recv_multipart_msg(_msg_, _from_, _to_) while(1){\
  zmq_msg_init(&(_msg_));\
  zmq_msg_recv(&(_msg_), (_from_), 0);\
  debug_log("Msg: [%zu]\n", zmq_msg_size(&(_msg_)));\
  int __more=zmq_msg_more(&(_msg_));\
  zmq_msg_send(&(_msg_), (_to_), __more?ZMQ_SNDMORE:0);\
  zmq_msg_close(&(_msg_));\
  if (!__more){\
    break;\
  }\
}


int main(){
  void *context=zmq_ctx_new();
  void *frontend=zmq_socket(context, ZMQ_ROUTER);
  void *backend=zmq_socket(context, ZMQ_DEALER);
  zmq_bind(frontend, "tcp://*:5559");
  zmq_bind(backend, "tcp://*:5560");

  zmq_pollitem_t items[]={
    {frontend, 0, ZMQ_POLLIN, 0},
    {backend, 0, ZMQ_POLLIN, 0}
  };

  while(1){
    zmq_msg_t messages;
    zmq_poll(items, 2, -1);

    if (items[0].revents & ZMQ_POLLIN){
      recv_multipart_msg(messages, frontend, backend);
    }
    if (items[1].revents & ZMQ_POLLIN){
      recv_multipart_msg(messages, backend, frontend);
    }
  }

  zmq_close(backend);
  zmq_close(frontend);
  zmq_ctx_destroy(context);
}
