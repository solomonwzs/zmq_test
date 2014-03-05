#include "../base.h"


int main(){
  void *context=zmq_ctx_new();
  void *receiver=zmq_socket(context, ZMQ_PULL);
  int rc=zmq_bind(receiver, "tcp://*:5558");
  assert(rc==0);

  void *controller=zmq_socket(context, ZMQ_PUB);
  rc=zmq_bind(controller, "tcp://*:5559");
  assert(rc==0);

  char string[256];
  s_recv(receiver, string, 256);

  int64_t start_time;
  s_clock(&start_time);

  int task_nbr;
  for (task_nbr=0; task_nbr<100; ++task_nbr){
    s_recv(receiver, string, 256);

    if (task_nbr%10==0){
      printf(":");
    } else{
      printf(".");
    }
    fflush(stdout);
  }

  int64_t end_time;
  s_clock(&end_time);

  printf("Total elapsed time: %d msec\n", (int)(end_time-start_time));

  s_send(controller, "KILL", 0, NULL);

  zmq_close(controller);
  zmq_close(receiver);
  zmq_ctx_destroy(context);

  return 0;
}
