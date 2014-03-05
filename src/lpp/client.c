#include <czmq.h>
#include "../base.h"

#define REQUEST_TIMEOUT 2500
#define REQUEST_RETRIES 3
#define SERVER_ENDPOINT "tcp://localhost:5555"


int main(){
  zctx_t *ctx=zctx_new();
  debug_log("I: connecting to server"STR_ELLIPSIS"\n");
  void *client=zsocket_new(ctx, ZMQ_REQ);
  assert(client);
  zsocket_connect(client, SERVER_ENDPOINT);

  int sequence=0;
  int retries_left=REQUEST_RETRIES;
  while (retries_left && !zctx_interrupted){
    char request[10];
    sprintf(request, "%d", ++sequence);
    zstr_send(client, request);

    int expect_reply=1;
    while (expect_reply){
      zmq_pollitem_t items[]={{client, 0, ZMQ_POLLIN, 0}};
      int rc=zmq_poll(items, 1, REQUEST_TIMEOUT*ZMQ_POLL_MSEC);
      if (rc==-1){
        break;
      }

      if (items[0].revents & ZMQ_POLLIN){
        char *reply=zstr_recv(client);
        if (!reply){
          break;
        }
        if (atoi(reply)==sequence){
          debug_log("I: server replied OK (%s)\n", reply);
          retries_left=REQUEST_RETRIES;
          expect_reply=0;
        } else{
          debug_log("E: malfored reply from server: %s\n", reply);
        }
        free(reply);
      } else if (--retries_left==0){
        debug_log("E: server seems to be offline, abandoning\n");
        break;
      }else{
        debug_log("W: no response from server, replying"STR_ELLIPSIS"\n");
        zsocket_destroy(ctx, client);
        debug_log("I: reconnecting to server"STR_ELLIPSIS"\n");

        client=zsocket_new(ctx, ZMQ_REQ);
        zsocket_connect(client, SERVER_ENDPOINT);

        zstr_send(client, request);
      }
    }
  }

  zctx_destroy(&ctx);
  return 0;
}
