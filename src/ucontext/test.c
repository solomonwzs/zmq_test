#include <ucontext.h>
#include "../base.h"

#define handle_error(msg) do{\
  perror(msg); \
  exit(EXIT_FAILURE); \
} while(0)

static ucontext_t uctx_main, uctx_func1, uctx_func2;


static void func1(){
  debug_log("func1: started\n");
  debug_log("func1: swapcontext(&uctx_func1, &uctx_func2)\n");
  if (swapcontext(&uctx_func1, &uctx_func2)==-1){
    handle_error("swapcontext");
  }
  debug_log("func1: running\n");
}


static void func2(){
  debug_log("func2: started\n");
  debug_log("func2: swapcontext(&uctx_func2, &uctx_func1)\n");
  if (swapcontext(&uctx_func2, &uctx_func1)==-1){
    handle_error("swapcontext");
  }
  debug_log("func2: running\n");
}


int main(int argc, char *argv[]){
  char func1_stack[16384];
  char func2_stack[16384];

  if (getcontext(&uctx_func1)==-1){
    handle_error("getcontext");
  }
  uctx_func1.uc_stack.ss_sp=func1_stack;
  uctx_func1.uc_stack.ss_size=sizeof(func1_stack);
  uctx_func1.uc_link=&uctx_main;
  makecontext(&uctx_func1, func1, 0);

  if (getcontext(&uctx_func2)==-1){
    handle_error("getcontext");
  }
  uctx_func2.uc_stack.ss_sp=func2_stack;
  uctx_func2.uc_stack.ss_size=sizeof(func2_stack);
  uctx_func2.uc_link=(argc>1)?NULL:&uctx_func1;
  makecontext(&uctx_func2, func2, 0);

  debug_log("main: swapcontext(&uctx_main, &uctx_func2)\n");
  swapcontext(&uctx_main, &uctx_func2);

  debug_log("main: exiting\n");
  exit(EXIT_SUCCESS);
}
