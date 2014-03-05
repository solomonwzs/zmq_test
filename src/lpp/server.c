#include "../base.h"

int main(){
  srandom((unsigned)time(NULL));

  void *context=zmq_ctx_new();
  void *server=zmq_socket(context, ZMQ_REP);
  zmq_bind(server, "tcp://*:5555");

  int cycles=0;
  while (1){
    char request[32];
    s_recv(server, request, 32);
    ++cycles;

    if (cycles>3 && randof(3)==0){
      debug_log("I: simulating a crash\n");
      break;
    } else if (cycles>3 && randof(3)==0){
      debug_log("I: simulating CPU overload\n");
      sleep(2);
    }

    debug_log("I: normal request (%s)\n", request);
    sleep(1);
    s_send(server, request, 0, NULL);
  }

  zmq_close(server);
  zmq_ctx_destroy(context);

  return 0;
}
