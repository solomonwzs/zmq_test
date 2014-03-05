#include "../base.h"

int main(){
  void *context=zmq_ctx_new();

  void *receiver=zmq_socket(context, ZMQ_PULL);
  zmq_connect(receiver, "tcp://localhost:5557");

  void *sender=zmq_socket(context, ZMQ_PUSH);
  zmq_connect(sender, "tcp://localhost:5558");

  void *controller=zmq_socket(context, ZMQ_SUB);
  zmq_connect(controller, "tcp://localhost:5559");
  zmq_setsockopt(controller, ZMQ_SUBSCRIBE, "", 0);

  zmq_pollitem_t items[]={
    {receiver, 0, ZMQ_POLLIN, 0},
    {controller, 0, ZMQ_POLLIN, 0}
  };

  char string[256];
  while (1){
    zmq_poll(items, 2, -1);
    if (items[0].revents & ZMQ_POLLIN){
      s_recv(receiver, string, 256);

      printf("%s.", string);
      fflush(stdout);

      s_sleep(atoi(string));
      s_send(sender, string, 0, NULL);
    }

    if (items[1].revents & ZMQ_POLLIN){
      break;
    }
  }

  zmq_close(controller);
  zmq_close(receiver);
  zmq_close(sender);
  zmq_ctx_destroy(context);

  return 0;
}
