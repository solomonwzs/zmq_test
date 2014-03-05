#ifndef ZMQTEST_PPQ_PPQ_H
#define ZMQTEST_PPQ_PPQ_H

#define HEARTBEAT_LIVENESS 3
#define HEARTBEAT_INTERVAL 1000

#define INTERVAL_INIT 1000
#define INTERVAL_MAX 32000

#define PPP_READY "\001"
#define PPP_HEARTBEAT "\002"

typedef struct{
  zframe_t *identity;
  char *id_string;
  int64_t expiry;
} worker_t;

#endif
