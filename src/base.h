#ifndef ZMPTEST_BASE_H
#define ZMPTEST_BASE_H

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <zmq.h>

#define STR_ELLIPSIS "\xe2\x80\xa6"

#define _DEBUG_TIME_INFO

#define _LOG_COLOR_NORMAL "\033[0m"
#define _LOG_COLOR_YELLOW "\033[0;33m"

#define _put_datetime(_buf_) do{\
  struct tm *__t;\
  time_t __time;\
  time(&__time);\
  __t=localtime(&__time);\
  strftime(_buf_, 32, "%F %T", __t);\
} while(0)

#if defined _DEBUG_TIME_FILE_INFO
#define debug_log(_fmt_, ...) do{\
  char __datetime[32];\
  _put_datetime(__datetime);\
  printf("%s [%s:%d] "_fmt_, __datetime, __FILE__, __LINE__, ## __VA_ARGS__);\
} while(0)
#elif defined _DEBUG_TIME_INFO
#define debug_log(_fmt_, ...) do{\
  char __datetime[32];\
  _put_datetime(__datetime);\
  printf(_LOG_COLOR_YELLOW"[%s]"_LOG_COLOR_NORMAL" "_fmt_, __datetime, ## __VA_ARGS__);\
} while(0)
#elif defined _DEBUG_FILE_INFO
#define debug_log(_fmt_, ...) \
  printf("[%s:%d] "_fmt_, __FILE__, __LINE__, ## __VA_ARGS__)
#elif defined _DEBUG_INFO
#define debug_log(_fmt_, ...) printf(_fmt_, ## __VA_ARGS__)
#else
#define debug_log(_fmt_, ...)
#endif


/*#define debug_log(_fmt_, ...) do {\
  char *__p=_LOG_FORMAT;\
  while (*__p){\
    if (*__p=='%'){\
      switch (*++__p){\
        case 't':\
          _print_time;\
          break;\
        case 'f':\
          printf(__FILE__);\
          break;\
        case 'l':\
          printf("%d", __LINE__);\
          break;\
        case 'c':\
          printf(_fmt_, ## __VA_ARGS__);\
          break;\
        default:\
          break;\
      }\
      ++__p;\
    } else{\
      putchar(*__p++);\
    }\
  }\
} while(0)*/

#ifndef randof
#define randof(_num_) (int)((float)(_num_)*random()/(RAND_MAX+1.0))
#endif

#define parser_errno(_err_) switch (_err_) {\
  case EAGAIN:\
    debug_log("EAGAIN\n");\
    break;\
  case ENOTSUP:\
    debug_log("ENOTSUP\n");\
    break;\
  case EFSM:\
    debug_log("EFSM\n");\
    break;\
  case ETERM:\
    debug_log("ETERM\n");\
    break;\
  case ENOTSOCK:\
    debug_log("ENOTSOCK\n");\
    break;\
  case EINTR:\
    debug_log("EINTR\n");\
    break;\
  default:\
    debug_log("UNKNOWN\n");\
    break;\
}


extern int S_INTERRUPTED;


extern void s_signal_handler(int signal_value);

extern void s_catch_signals();

extern void s_recv(void *socket, char *str, unsigned len);

extern void s_send(void *socket, char *str, int len, int *size);

extern void s_sendmore(void *socket, char *str, int len, int *size);

extern void s_sendlists(void *socket, unsigned count, ...);

extern void s_sleep(int msecs);

extern void s_clock(int64_t *msecs);

extern void s_dump(void *socket);

extern void s_set_id(void *socket);

#endif
