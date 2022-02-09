
CFLAGS=-Wall -Werror -DC99 -std=gnu99 -D_FORTIFY_SOURCE=2

all:    
	gcc  $(CFLAGS) -o sslconnect sslconnect.c -lssl -lcrypto   


