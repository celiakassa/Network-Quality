#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>


#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>

SSL_CTX* InitCTX(void){ 
  BIO              *certbio = NULL;
  BIO               *outbio = NULL;
  const SSL_METHOD *method;
  SSL_CTX *ctx;
  
  /* These function calls initialize openssl for correct work.  */
   
  OpenSSL_add_all_algorithms();
  ERR_load_BIO_strings();
  ERR_load_crypto_strings();
  SSL_load_error_strings();
   
   /* Create the Input/Output BIO's.                             */
   
  certbio = BIO_new(BIO_s_file());
  outbio  = BIO_new_fp(stdout, BIO_NOCLOSE);

  /* initialize SSL library and register algorithms             */   
  if(SSL_library_init() < 0)
    BIO_printf(outbio, "Could not initialize the OpenSSL library !\n");
   
   /* Set SSLv2 client hello, also announce SSLv3 and TLSv1      */  
  method = SSLv23_client_method();
  
   /* Try to create a new SSL context                            */   
  if ( (ctx = SSL_CTX_new(method)) == NULL)
    BIO_printf(outbio, "Unable to create a new SSL context structure.\n");

  /* Disabling SSLv2 will leave v3 and TSLv1 for negotiation    */   
  SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2);
  return ctx;
}

/*Show  Certificate Information*/
void ShowCerts(SSL* ssl ,BIO* outbio)
{  
   X509 *cert;
   X509_NAME       *certname ;
  // char *line;
   
 /* Get the remote certificate into the X509 structure         */
    cert = SSL_get_peer_certificate(ssl);  
     if (cert == NULL)
   	 BIO_printf(outbio, "Error: Could not get a certificate from server.\n");
    else
   	 BIO_printf(outbio, "Retrieved the server's certificate from server.\n");
   	
   /* extract various certificate information                    */
    certname = X509_NAME_new();
    certname = X509_get_subject_name(cert);

	/* display the cert subject here                              */
    BIO_printf(outbio, "Displaying the certificate subject data:\n");
    X509_NAME_print_ex(outbio, certname, 0, 0);
    BIO_printf(outbio, "\n");

}
 
/* ---------------------------------------------------------- *
 * First we need to make a standard TCP socket connection.    *
 * create_socket() creates a socket & TCP-connects to server. *
 * ---------------------------------------------------------- */
int create_socket(char[], BIO *);

int main(int argc, char *argv[]) {

  BIO              *certbio = NULL;
  BIO               *outbio = NULL;
  X509                *cert = NULL;
  X509_NAME       *certname = NULL;
  const SSL_METHOD *method;
  SSL_CTX *ctx;
  SSL *ssl;
  int server = 0;
  int bytes;
  int ret, i;
  char buf[1024];
  char *Request = (char*) malloc(1024);
  
  // for async reading
  int fdmax = 0;
  fd_set readfds;
  
  if(argc!=3){
     printf("Usage %s <url> <resources>\n", argv[0]);
     return -1;
  }
  
  char *dest_url = argv[1]; //"https://monitor.uac.bj:4449";
  char *request = argv[2];
  ctx = InitCTX();  
  ssl = SSL_new(ctx);/* Create new SSL connection state object */
  server = create_socket(dest_url, outbio);/* Make the underlying TCP socket connection*/ 
  if(server != 0)
     BIO_printf(outbio, "Successfully made the TCP connection to: %s.\n", dest_url);
  SSL_set_fd(ssl, server); /* Attach the SSL session to the socket descriptor*/
  if ( SSL_connect(ssl) != 1 )/* Try to SSL-connect here, returns 1 for success   */
    BIO_printf(outbio, "Error: Could not build a SSL session to: %s.\n", dest_url);
  else{
    BIO_printf(outbio, "Successfully enabled SSL/TLS session to: %s.\n", dest_url);
    
    }
  
  ShowCerts(ssl,outbio);
  // make socket NON Blocking 
  
  fcntl(server, F_SETFL, O_NONBLOCK );
  FD_ZERO(&readfds);
  fdmax = server+1;
  int exit = 0;
  const char *cpRequestMessage = "GET /%s HTTP/1.1\nHost: monitor.uac.bj\n\n";
  char *requestMsg = (char*) malloc(60);
  sprintf(requestMsg, cpRequestMessage, request);
  if(SSL_write(ssl, requestMsg, strlen(requestMsg)) < 0){
   	printf("\n\n Echec Envoie de requete\n");
   	return -1;
  }
    
  printf("\n\nEnvoie de requete reussie\n");
  while(!exit){
      FD_SET(server, &readfds);
      ret = select(fdmax, &readfds, NULL, NULL, NULL);
      if(FD_ISSET(server, &readfds)){
         while(((bytes = SSL_read(ssl, buf, 1024)) < 0) && (errno == EAGAIN));
         printf("%s %d", buf, bytes);
         //exit = 1;
         printf("%d %d\n", bytes, errno);
         FD_CLR(server, &readfds);
      }
  }
   
  printf("after while\n");
   
   
  /* ---------------------------------------------------------- *
   * Free the structures we don't need anymore                  *
   * -----------------------------------------------------------*/
  SSL_free(ssl);
  close(server);
  X509_free(cert);
  SSL_CTX_free(ctx);
  BIO_printf(outbio, "Finished SSL/TLS connection with server: %s.\n", dest_url);
  return(0);
}

/* ---------------------------------------------------------- *
 * create_socket() creates the socket & TCP-connect to server *
 * ---------------------------------------------------------- */
int create_socket(char url_str[], BIO *out) {
  int sockfd;
  char hostname[256] = "";
  char    portnum[6] = "443";
  char      proto[6] = "";
  char      *tmp_ptr = NULL;
  int           port;
  struct hostent *host;
  struct sockaddr_in dest_addr;

  /* ---------------------------------------------------------- *
   * Remove the final / from url_str, if there is one           *
   * ---------------------------------------------------------- */
  if(url_str[strlen(url_str)] == '/')
    url_str[strlen(url_str)] = '\0';

  /* ---------------------------------------------------------- *
   * the first : ends the protocol string, i.e. http            *
   * ---------------------------------------------------------- */
  strncpy(proto, url_str, (strchr(url_str, ':')-url_str));

  /* ---------------------------------------------------------- *
   * the hostname starts after the "://" part                   *
   * ---------------------------------------------------------- */
  strncpy(hostname, strstr(url_str, "://")+3, sizeof(hostname));

  /* ---------------------------------------------------------- *
   * if the hostname contains a colon :, we got a port number   *
   * ---------------------------------------------------------- */
  if(strchr(hostname, ':')) {
    tmp_ptr = strchr(hostname, ':');
    /* the last : starts the port number, if avail, i.e. 8443 */
    strncpy(portnum, tmp_ptr+1,  sizeof(portnum));
    *tmp_ptr = '\0';
  }

  port = atoi(portnum);
  // TODO BIO est il vraiment nécessaire???
  if ( (host = gethostbyname(hostname)) == NULL ) {
    BIO_printf(out, "Error: Cannot resolve hostname %s.\n",  hostname);
    abort();
  }

  /* ---------------------------------------------------------- *
   * create the basic TCP socket                                *
   * ---------------------------------------------------------- */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  dest_addr.sin_family=AF_INET;
  dest_addr.sin_port=htons(port);
  dest_addr.sin_addr.s_addr = *(long*)(host->h_addr);

  /* ---------------------------------------------------------- *
   * Zeroing the rest of the struct                             *
   * ---------------------------------------------------------- */
  memset(&(dest_addr.sin_zero), '\0', 8);

  tmp_ptr = inet_ntoa(dest_addr.sin_addr);

  /* ---------------------------------------------------------- *
   * Try to make the host connect here                          *
   * ---------------------------------------------------------- */
  if ( connect(sockfd, (struct sockaddr *) &dest_addr,
                              sizeof(struct sockaddr)) == -1 ) {
    BIO_printf(out, "Error: Cannot connect to host %s [%s] on port %d.\n",
             hostname, tmp_ptr, port);
  }

  return sockfd;
}
