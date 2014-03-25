#include "../erl_inc.h"

#define check_allocated(_a_, _f_) do{\
  erl_eterm_statistics(&_a_, &_f_);\
  debug_log("allocated: %ld, freed: %ld\n", _a_, _f_);\
} while(0)

#define BUFFER_SIZE 64*1024


int main(){
  void *context=zmq_ctx_new ();
  void *responder=zmq_socket (context, ZMQ_REP);
  int rc=zmq_bind (responder, "tcp://*:5555");
  assert(rc==0);

  while (1){
    char buffer[BUFFER_SIZE];
    s_recv(responder, buffer, BUFFER_SIZE);

    erl_init(NULL, 0);
    ETERM *et=erl_decode((unsigned char *)buffer);
    erl_free_term(et);
    erl_eterm_release();

    //debug_log("%s\n", buffer);

    s_send(responder, "ok", 0, NULL);
  }
}


//int main(){
//  void *context=zmq_ctx_new ();
//  void *responder=zmq_socket (context, ZMQ_REP);
//  int rc=zmq_bind (responder, "tcp://*:5555");
//  assert(rc==0);
//
//  erl_init(NULL, 0);
//
//  char buffer[256];
//  char encode[256];
//  ETERM *tuple, *arr[2];
//  int index, type, size, len;
//  unsigned long allocated, freed;
//  while (1){
//    debug_log("ok\n");
//    s_recv(responder, buffer, 256);
//
//    index=1;
//    //ei_decode_string((const char *)buffer, &index, encode_str);
//    //debug_log("Received [%s]\n", encode_str);
//    ei_get_type((const char *)buffer, &index, &type, &size);
//    debug_log("type: %c, size: %d\n", type, size);
//
//    check_allocated(allocated, freed);
//
//    //erl_eterm
//    arr[0]=erl_mk_atom("Good");
//    arr[1]=erl_mk_int(1024);
//    tuple=erl_mk_tuple(arr, 2);
//    check_allocated(allocated, freed);
//
//    len=erl_encode(tuple, (unsigned char *)encode);
//    erl_free_compound(tuple);
//    check_allocated(allocated, freed);
//
//    debug_log("len: %zu\n", strlen(encode));
//
//    sleep (1);
//    s_send (responder, encode, len, NULL);
//  }
//  return 0;
//}
