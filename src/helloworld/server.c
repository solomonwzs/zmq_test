#include "../base.h"

int main (void)
{
  void *context=zmq_ctx_new ();
  void *responder=zmq_socket (context, ZMQ_REP);
  int rc=zmq_bind (responder, "tcp://*:5555");
  assert(rc==0);

  s_catch_signals();
  char buffer[10];
  while (!S_INTERRUPTED){
    zmq_recv(responder, buffer, 10, 0);

    if (S_INTERRUPTED){
      debug_log("Interrupt received, killing server\n");
      break;
    }

    debug_log("Received: %s\n", buffer);
    sleep (1);
    zmq_send (responder, "World", 5, 0);
  }
  return 0;
}
