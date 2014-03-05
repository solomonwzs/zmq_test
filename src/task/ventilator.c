#include "../base.h"

int main(){
  void *context=zmq_ctx_new();

  void *sender=zmq_socket(context, ZMQ_PUSH);
  zmq_bind(sender, "tcp://*:5557");

  void *sink=zmq_socket(context, ZMQ_PUSH);
  zmq_connect(sink, "tcp://localhost:5558");

  printf("Press Enter when the workers are ready: ");
  getchar();
  printf("Sending tasks to workers...\n");

  s_send(sink, (char *)"0", 0, NULL);

  srandom((unsigned)time(NULL));

  int task_nbr;
  int total_msec=0;
  for (task_nbr=0; task_nbr<100; ++task_nbr){
    int workload;

    workload=randof(100)+1;
    total_msec+=workload;

    char string[10];
    sprintf(string, "%d", workload);
    s_send(sender, string, 0, NULL);
  }
  printf("Total expected cost: %d msecs\n", total_msec);

  zmq_close(sink);
  zmq_close(sender);
  zmq_ctx_destroy(context);

  return 0;
}
