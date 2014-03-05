#include "../base.h"


static int f(){
  static int i, state=0;
  switch (state){
    case 0:
      for (i=0; i<2; ++i){
        state=1;
        return i;
        case 1:
        ;
      }
  }
}


int main(){
  debug_log("f: %d\n", f());
  debug_log("f: %d\n", f());
  debug_log("f: %d\n", f());
  debug_log("f: %d\n", f());

  return 0;
}
