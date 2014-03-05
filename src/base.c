#include "base.h"


int S_INTERRUPTED=0;


void s_signal_handler(int signal_value){
  debug_log("pid:%d, %d, %s\n", getpid(), signal_value, strsignal(signal_value));
  S_INTERRUPTED=1;
}


void s_catch_signals(){
  struct sigaction action;
  action.sa_handler=s_signal_handler;
  action.sa_flags=0;
  sigemptyset(&action.sa_mask);
  sigaction(SIGINT, &action, NULL);
  sigaction(SIGTERM, &action, NULL);
}


void s_recv(void *socket, char *str, unsigned len){
  int size=zmq_recv(socket, str, len-1, 0);
  if (size==-1){
    parser_errno(errno);
    return;
  }
  if (size>len-1){
    size=len-1;
  }

  str[size]=0;
}


void s_send(void *socket, char *str, int len, int *size){
  int l=len?len:strlen(str);
  int s=zmq_send(socket, str, l, 0);

  if (s==-1){
    parser_errno(errno);
  }

  if (size!=NULL){
    *size=s;
  }
}


void s_sendmore(void *socket, char *str, int len, int *size){
  int l=len?len:strlen(str);
  int s=zmq_send(socket, str, l, ZMQ_SNDMORE);

  if (s==-1){
    parser_errno(errno);
  }

  if (size!=NULL){
    *size=s;
  }
}


void s_sleep(int msecs){
  struct timespec t;
  t.tv_sec=msecs/1000;
  t.tv_nsec=(msecs%1000)*1000000;

  nanosleep(&t, NULL);
}


void s_clock(int64_t *msecs){
  struct timeval tv;
  gettimeofday(&tv, NULL);

  *msecs=(int64_t)(tv.tv_sec*1000+tv.tv_usec/1000);
}


void s_dump(void *socket){
  debug_log("----------\n");
  while (1){
    zmq_msg_t msg;
    zmq_msg_init(&msg);
    int size=zmq_msg_recv(&msg, socket, 0);

    char *data=zmq_msg_data(&msg);
    int is_text=1;
    int char_nbt;
    for (char_nbt=0; char_nbt<size; ++char_nbt){
      if ((unsigned char)data[char_nbt]<32 ||
          (unsigned char)data[char_nbt]>127){
        is_text=0;
        break;
      }
    }

    debug_log("[%03d] ", size);
    for (char_nbt=0; char_nbt<size; ++char_nbt){
      if (is_text){
        printf("%c", data[char_nbt]);
      } else{
        printf("%02X", (unsigned char)data[char_nbt]);
      }
    }
    printf("\n");

    int64_t more=0;
    size_t more_size=sizeof(more);
    zmq_getsockopt(socket, ZMQ_RCVMORE, &more, &more_size);
    zmq_msg_close(&msg);
    if (!more){
      break;
    }
  }
}


void s_set_id(void *socket){
  char id[16];
  sprintf(id, "%04X-%04X", randof(0x10000), randof(0x10000));
  zmq_setsockopt(socket, ZMQ_IDENTITY, id, strlen(id));
}


void s_sendlists(void *socket, unsigned count, ...){
  va_list ap;
  char *data;
  int len;
  unsigned i;

  va_start(ap, count);
  for (i=0; i<count-1; i++){
    data=va_arg(ap, char *);
    len=va_arg(ap, int);
    zmq_send(socket, data, len?len:strlen(data), ZMQ_SNDMORE);
  }
  data=va_arg(ap, char *);
  len=va_arg(ap, int);
  zmq_send(socket, data, len?len:strlen(data), 0);

  va_end(ap);
}
