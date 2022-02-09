
all:    
	gcc -Wall -Werror -DC99 -std=gnu99 -o sslconnect sslconnect.c -lssl -lcrypto   


