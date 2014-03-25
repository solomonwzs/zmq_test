#ifndef __ZMQTEST_API_MDP_MDWRKAPI_H__
#define __ZMQTEST_API_MDP_MDWRKAPI_H__

#include "mdp.h"

typedef struct{
  zctx_t *ctx;
  char *broker;
  char *service;
  void *worker;
  int verbose;

  uint64_t heartbeat_at;
  size_t liveness;
  int heartbeat;
  int reconnect;

  int expect_reply;
  zframe_t *reply_to;
} mdwrk_t;

extern mdwrk_t *mdwrk_new(char *broker, char *service, int verbose);

extern void mdwrk_destroy(mdwrk_t **self_p);

extern zmsg_t *mdwrk_recv(mdwrk_t *self, zmsg_t **reply_p);

#define mdwrk_set_heartbeat(_self_, _heartbeat_) \
    (_self_)->heartbeat=(_heartbeat_)

#define mdwrk_set_reconnect(_self_, _reconnect_) \
    (_self_)->reconnect=(_reconnect_)

#endif
