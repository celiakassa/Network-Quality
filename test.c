#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include "test.h"


long get_duration(struct timespec a, struct timespec b) {
   long anano = 1000000000L * a.tv_sec + a.tv_nsec;
   long bnano = 1000000000L * b.tv_sec + b.tv_nsec;
   return anano - bnano;
}

long dns(char url_str[]){
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int  s;
  struct timespec ts_dns_start, ts_dns_end, ts_dns_result;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;    
  hints.ai_socktype = SOCK_DGRAM; 
  hints.ai_flags = 0;
  hints.ai_protocol = 0;          
  timespec_get(&ts_dns_start, TIME_UTC);
  if((s = getaddrinfo(url_str, NULL,&hints, &result)) != 0){
     fprintf(stderr, "An error occured %s\n", strerror(errno));
     return -1L;
  }
  timespec_get(&ts_dns_end, TIME_UTC);
  return get_duration(ts_dns_end,ts_dns_start);          
}


long tcp(char url_str[]){
  int sock;
  int port = 443;
  struct sockaddr_in sockaddrin;
  struct hostent *host;
  struct timespec ts_dns_start, ts_dns_end, ts_dns_result;
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if(sock == -1) 
     return -1L;
  host = gethostbyname(url_str);
  if(host == NULL) 
    return -1L;
  bcopy(host->h_addr, &sockaddrin.sin_addr, host->h_length);
  sockaddrin.sin_family = AF_INET;
  sockaddrin.sin_port = htons(port);
  timespec_get(&ts_dns_start, TIME_UTC);
  if(connect(sock, (struct sockaddr*) &sockaddrin, sizeof(struct sockaddr_in)) != 0)
     return -1L;
  
  timespec_get(&ts_dns_end, TIME_UTC);
  return get_duration(ts_dns_end,ts_dns_start);
}
