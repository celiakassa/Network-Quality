#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/conf.h>
#include <time.h>

struct config_urls {
 	char *SmallUrl    ;
	char *LargeUrl  ;
	char *UploadUrl ;  
};

typedef struct config_urls Configurls;

Configurls configuration(char *argv[]);

int OpenConnection(const char *hostname, int port);
SSL_CTX* InitCTX(void);
void ShowCerts(SSL* ssl);
