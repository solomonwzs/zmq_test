#include "../base.h"

#define NBR_WORKERS 3


static void *worker_task(void *args){
  void *context=zmq_ctx_new();
  void *worker=zmq_socket(context, ZMQ_REQ);
  s_set_id(worker);
  zmq_connect(worker, "tcp://localhost:5671");

  int total=0;
  pthread_t tid=pthread_self();
  char msg[256];
  while (1){
    //s_sendmore(worker, "", 0, NULL);
    s_send(worker, "Hello", 0, NULL);

    s_recv(worker, msg, 256);
    debug_log("[%X] receive: %s\n", (int)tid, msg);

    //s_recv(worker, msg, 256);
    //debug_log("[%X] receive: %s\n", tid, msg);

    int finished=(strcmp(msg, "exit")==0);
    if (finished){
      printf("Completed: %d tasks\n", total);
      break;
    }
    ++total;

    s_sleep(randof(500)+1);
  }

  zmq_close(worker);
  zmq_ctx_destroy(context);
  return NULL;
}


int main(){
  void *context=zmq_ctx_new();
  void *broker=zmq_socket(context, ZMQ_ROUTER);

  zmq_bind(broker, "tcp://*:5671");
  srandom((unsigned)time(NULL));

  int worker_nbr;
  for (worker_nbr=0; worker_nbr<NBR_WORKERS; ++worker_nbr){
    pthread_t worker;
    pthread_create(&worker, NULL, worker_task, NULL);
  }

  int64_t end_time;
  int workers_exited=0;
  char str[256];
  s_clock(&end_time);
  end_time+=500;
  while (1){
    s_recv(broker, str, 256);
    s_sendmore(broker, str, 0, NULL);

    s_recv(broker, str, 256);
    debug_log(">  receive: %s\n", str);
    s_recv(broker, str, 256);
    debug_log(">> receive: %s\n", str);
    s_sendmore(broker, "", 0, NULL);

    int64_t now;
    s_clock(&now);
    if (now<end_time){
      s_send(broker, "ok", 0, NULL);
    } else{
      s_send(broker, "exit", 0, NULL);
      if (++workers_exited==NBR_WORKERS){
        break;
      }
    }
  }

  zmq_close(broker);
  zmq_ctx_destroy(context);
  return 0;
}
