#ifndef __ZMQTEST_API_MDP_MDP_H__
#define __ZMQTEST_API_MDP_MDP_H__

#include <czmq.h>
#include "../../src/base.h"

#define MDPC_CLIENT "MDPC01"
#define MDPW_WORKER "MDPW01"

#define MDPW_READY "\001"
#define MDPW_REQUEST "\002"
#define MDPW_REPLY "\003"
#define MDPW_HEARTBEAT "\004"
#define MDPW_DISCONNECT "\005"

#define WARN_COLOR "\033[01;33m"
#define ERROR_COLOR "\033[01;31m"
#define NORMAL_COLOR "\033[0m"

static char *mdps_commands[]={
  NULL, "READY", "REQUEST", "REPLY", "HEARTBEAT", "DISCONNECT"
};

#endif
