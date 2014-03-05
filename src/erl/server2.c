#include "../erl_inc.h"

#define check_allocated(_a_, _f_) do{\
  erl_eterm_statistics(&_a_, &_f_);\
  debug_log("allocated: %ld, freed: %ld\n", _a_, _f_);\
} while(0)

#define response_tuple(_msg_, _arr_, _tuple_) do{\
  _arr_[0]=erl_mk_int(getpid());\
  _arr_[1]=erl_mk_string(_msg_);\
  _tuple_=erl_mk_tuple(_arr_, 2);\
} while(0)

#define send_tuple(_responder_, _msg_) do{\
  ETERM *__tuple, *__arr[2];\
  int __len;\
  char __encode[256];\
  __arr[0]=erl_mk_int(getpid());\
  __arr[1]=erl_mk_string(_msg_);\
  __tuple=erl_mk_tuple(__arr, 2);\
  __len=erl_encode(__tuple, (unsigned char *)__encode);\
  erl_free_compound(__tuple);\
  s_send((_responder_), __encode, __len, NULL);\
} while(0)


int main()
{
  void *context=zmq_ctx_new ();
  void *responder=zmq_socket (context, ZMQ_PULL);
  int rc=zmq_bind (responder, "tcp://*:5555");
  assert(rc==0);

  erl_init(NULL, 0);

  char buffer[256];
  //char encode[256];
  //ETERM *tuple, *arr[2];
  int index, type, size;
  pid_t child_pid;
  //unsigned long allocated, freed;
  s_catch_signals();
  while (1){
    s_recv(responder, buffer, 256);

    index=1;
    ei_get_type((const char *)buffer, &index, &type, &size);

    if (type==ERL_SMALL_INTEGER_EXT){
      child_pid=fork();

      if (child_pid==0){
        debug_log("fork child: %d\n", getpid());
        sleep(10);
        //send_tuple(responder, "integer");
        //signal(SIGCHLD, SIG_IGN);
        //debug_log("%d, %d\n", allocated, freed);
        return 0;
      } else{
        waitpid(-1, 0, WNOHANG);
        //debug_log("%d, %d\n", allocated, freed);
        //send_tuple(responder, "integer-");
      }
      debug_log("receive integer\n");
    } else if (type==ERL_FLOAT_EXT){
      debug_log("receive float\n");
      //send_tuple(responder, "float");
    } else{
      //send_tuple(responder, "other");
    }

    if (S_INTERRUPTED){
      debug_log("Interrupt received, killing server\n");
      break;
    }
    sleep(1);
  }

  zmq_close(responder);
  zmq_ctx_destroy(context);

  return 0;
}
