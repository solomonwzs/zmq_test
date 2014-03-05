#include "../base.h"

int main(int argc, char *argv[]){
  printf("Collecting updates from weather server...\n");
  void *context=zmq_ctx_new();
  void *subscriber=zmq_socket(context, ZMQ_SUB);
  int rc=zmq_connect(subscriber, "tcp://localhost:5556");
  assert(rc==0);

  char *filter=(argc>1)?argv[1]:(char *)"10001";
  rc=zmq_setsockopt(subscriber, ZMQ_SUBSCRIBE, filter, strlen(filter));
  assert(rc==0);

  int update_nbr;
  long total_temp=0;
  char string[256];
  int zipcode, temperature, relhumidity;
  for (update_nbr=0; update_nbr<100; ++update_nbr){
    s_recv(subscriber, string, 256);

    sscanf(string, "%d %d %d", &zipcode, &temperature, &relhumidity);
    total_temp+=temperature;
    printf(".");
    fflush(stdout);
  }
  printf("\n");
  printf("Average temperature for zipcode '%s' was %dF\n", filter,
         (int)(total_temp/update_nbr));

  zmq_close(subscriber);
  zmq_ctx_destroy(context);

  return 0;
}
