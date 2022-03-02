#include <sys/types.h> 
#include <sys/socket.h> 
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void timespec_diff2(struct timespec *a, struct timespec *b,
    struct timespec *result) {
    result->tv_sec  = a->tv_sec  - b->tv_sec;
    result->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (result->tv_nsec < 0) {
        --result->tv_sec;
        result->tv_nsec += 1000000000L;
    }
}

int dns(char url_str[]){
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int  s;
 struct timespec ts_dns_start, ts_dns_end, ts_dns_result;
  /* Obtain address(es) matching host/port. */

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */
    timespec_get(&ts_dns_start, TIME_UTC);
   if((s = getaddrinfo(url_str, NULL,&hints, &result)) == 0){;
   	timespec_get(&ts_dns_end, TIME_UTC);
       timespec_diff2(&ts_dns_end,&ts_dns_start, &ts_dns_result);
    	//printf("%.3lf;", (ts_dns_result.tv_nsec/(double) 1000000));
    	return (ts_dns_result.tv_nsec/(double) 1000000);
    	}
   if (s != 0) {
               fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
               exit(EXIT_FAILURE);
   }
            
}



int main(int argc, char *argv[]){
 
 
 //dns("www.google.com");
 printf("DNS mesure\n");
 printf("%d\n",dns("www.uclouvain.be"));
 printf("%d\n",dns("www.uac.bj"));
 printf("%d\n",dns("www.emes.bj"));
 printf("%d\n",dns("www.github.com"));
 printf("%d\n",dns("www.rfc-editor.org."));
 


}
