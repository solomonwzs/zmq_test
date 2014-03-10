#include <czmq.h>
#include "../base.h"
#include "ppq.h"


static void *_worker_socket(zctx_t *ctx){
  void *worker=zsocket_new(ctx, ZMQ_DEALER);
  zsocket_connect(worker, "tcp://localhost:5556");

  debug_log("I: worker ready\n");
  zframe_t *frame=zframe_new(PPP_READY, 1);
  zframe_send(&frame, worker, 0);

  return worker;
}


int main(void){
  zctx_t *ctx=zctx_new();
  void *worker=_worker_socket(ctx);

  size_t liveness=HEARTBEAT_LIVENESS;
  size_t interval=INTERVAL_INIT;

  uint64_t heartbeat_at=zclock_time()+HEARTBEAT_INTERVAL;

  srandom((unsigned)time(NULL));
  int cycles=0;
  while (true){
    zmq_pollitem_t items[]={{worker, 0, ZMQ_POLLIN, 0}};
    int rc=zmq_poll(items, 1, HEARTBEAT_INTERVAL*ZMQ_POLL_MSEC);
    if (rc==-1){
      break;
    }

    if (items[0].revents & ZMQ_POLLIN){
      zmsg_t *msg=zmsg_recv(worker);
      if (!msg){
        break;
      }

      if (zmsg_size(msg)==3){
        ++cycles;
        if (cycles>3 && randof(5)==0){
          debug_log("I: simulating a crash\n");
          zmsg_destroy(&msg);
          break;
        } else if (cycles>3 && randof(5)==0){
          debug_log("I: simulating CPU overload\n");
          sleep(3);
          if (zctx_interrupted){
            break;
          }
        } else{
          debug_log("I: normal reply\n");
          zmsg_send(&msg, worker);
          sleep(1);
          if (zctx_interrupted){
            break;
          }
        }
      } else if (zmsg_size(msg)==1){
        zframe_t *frame=zmsg_first(msg);
        if (memcmp(zframe_data(frame), PPP_HEARTBEAT, 1)==0){
          liveness=HEARTBEAT_LIVENESS;
        } else{
          debug_log(ERROR_COLOR"E: inval message\n"NORMAL_COLOR);
          zmsg_dump(msg);
        }
        zmsg_destroy(&msg);
      } else{
        debug_log(ERROR_COLOR"E: invalid message\n"NORMAL_COLOR);
        zmsg_dump(msg);
      }
      interval=INTERVAL_INIT;
    } else if (--liveness==0){
      debug_log(WARN_COLOR"W: heartbeat failure, can't reach queue\n"
                NORMAL_COLOR);
      debug_log(WARN_COLOR"W: reconnecting in %zd msec"STR_ELLIPSIS"\n"
                NORMAL_COLOR, interval);
      zclock_sleep(interval);

      if (interval<INTERVAL_MAX){
        interval*=2;
      }

      zsocket_destroy(ctx, worker);
      worker=_worker_socket(ctx);
      liveness=HEARTBEAT_LIVENESS;
    }

    if (zclock_time()>heartbeat_at){
      heartbeat_at=zclock_time()+HEARTBEAT_INTERVAL;
      debug_log("I: worker heartbeat\n");
      zframe_t *frame=zframe_new(PPP_HEARTBEAT, 1);
      zframe_send(&frame, worker, 0);
    }
  }
  
  zctx_destroy(&ctx);
  return 0;
}
