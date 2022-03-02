#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <strings.h>


int establish_tcp(int port) {
    char *ip = "127.0.0.1"; /* Virtual Machine */
    struct sockaddr_in sockaddrin;
    struct hostent *host;
    int sock;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock == -1) {
        fprintf(stderr, "Socket Error\n");
        exit(1);
    }

    sockaddrin.sin_family = AF_INET;

    host = gethostbyname(ip);
    if(host == NULL) {
        fprintf(stderr, "%s unknown host.\n", ip);
        exit(2);
    }

    /* copies the internet address into the struct */
    bcopy(host->h_addr, &sockaddrin.sin_addr, host->h_length);

    /* specify port (used by TCP a layer above */
    sockaddrin.sin_port = htons(port);

    /* try to connect */
    return connect(sock, (struct sockaddr*) &sockaddrin, sizeof(struct sockaddr_in));
}

int main(void) {
    printf("status: %i\n", establish_tcp(8000));
    return 0;
}
