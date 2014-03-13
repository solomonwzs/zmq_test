#ifndef __ZMQTEST_API_MDP_MDCLIAPI_H__
#define __ZMQTEST_API_MDP_MDCLIAPI_H__

#include "mdp.h"

typedef struct{
  zctx_t *ctx;
  char *broker;
  void *client;
  int verbose;
  int timeout;
  int retries;
} mdcli_t;


extern mdcli_t *mdcli_new(char *broker, int verbose);

extern void mdcli_destroy(mdcli_t **self_p);

extern void mdcli_set_timeout(mdcli_t *self, int timeout);

extern void mdcli_set_retries(mdcli_t *self, int retries);

extern zmsg_t *mdcli_send(mdcli_t *self, char *server, zmsg_t **request_p);

#endif
