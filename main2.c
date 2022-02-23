 #include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

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
 int connexion(SSL* ssl ){
	  int bytes;
	  int ret, i;
	  char buf[1024];
	  char *Request = (char*) malloc(1024);
	  
	   const char *Message = "GET /config HTTP/1.1\nHost: monitor.uac.bj\n\n";
	   //sprintf(Request, cpRequestMessage);
	   if(SSL_write(ssl, Message, strlen(Message)) < 0){
	   	printf("\n\n Echec Envoie de requete\n");
	   	return -1;
	   }
	    
	   printf("\n\nEnvoie de requete reussie\n");
	   if((bytes = SSL_read(ssl, buf, 1024)) < 0){
	      printf("Rien n'a été reçu: \n");
	      return -1;
	   }
	   //bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
	   printf("Received something: %d\n", bytes);
	   buf[bytes] = '\0';
	   printf("2 Received: \"%s\"\n", buf);
	   
	   return(0);
 }
 
 

/* ---------------------------------------------------------- *
 * First we need to make a standard TCP socket connection.    *
 * create_socket() creates a socket & TCP-connects to server. *
 * ---------------------------------------------------------- */
int create_socket(char[], BIO *);

int main(int argc, char *argv[]) {

  char           dest_url[] = "https://monitor.uac.bj:4449";
  BIO              *certbio = NULL;
  BIO               *outbio = NULL;
  X509                *cert = NULL;
  X509_NAME       *certname = NULL;
  
  SSL_CTX *ctx;
  SSL *ssl;
  int server = 0;
  int bytes;
  int ret, i;
  char buf[1024];
  char *Request = (char*) malloc(1024);
   
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
   /* send request here                             */
  
   const char *cpRequestMessage = "GET /config HTTP/1.1\nHost: monitor.uac.bj\n\n";
   //sprintf(Request, cpRequestMessage);
   if(SSL_write(ssl, cpRequestMessage, strlen(cpRequestMessage)) < 0){
   	printf("\n\n Echec Envoie de requete\n");
   	return -1;
   }
    
   printf("\n\nEnvoie de requete reussie\n");
   if((bytes = SSL_read(ssl, buf, 1024)) < 0){
      printf("Rien n'a été reçu: \n");
      return -1;
   }
   //bytes = SSL_read(ssl, buf, sizeof(buf)); /* get reply & decrypt */
   printf("Received something: %d\n", bytes);
   buf[bytes] = '\0';
   printf("Received: \"%s\"\n", buf);
   
	/*Algorithme de saturation*/   
   int pfd[2];
  
   pid_t pid;
   fd_set readset, writeset;
   char buff[BUFFSIZE];
   char buffer[BUFFSIZE];
   int ret, ret_sel,fd_max, timeout = 0, bytestosend = 0;
   int offset = 0, bytesrecved = 0;
   int should_write = 0;
   sigset_t pselect_set;
   struct timeval out_time;
   fcntl(0, F_SETFL, O_NONBLOCK );
   fcntl(1, F_SETFL, O_NONBLOCK );
   FD_ZERO(&readset);
   FD_ZERO(&writeset);
   memset(buffer, 0, BUFFSIZE);
   
   
   /*Créer un processus fils pour lire dans notre pipe*/
   if ((pid = fork()) == -1){
      fprintf(stderr, "Echec de la création du processus fils\n");
      return 1;
   }
   if (pid == 0){
      long bytesrecv = 0;
      long byteswrite = 0;
      fd_max = 0;
      pid_t ppid = getppid();
      close(pfd[1]); //fermer l'extrémité de lecture pour éviter des comportements bizarres

      while (1){
         FD_SET(1, &writeset);
         fd_max = 1;
         FD_SET(pfd[0], &readset);
         if (pfd[0] > fd_max)
            fd_max = pfd[0];
         ret_sel = select(fd_max+1, &readset, &writeset, NULL, &out_time);

         if (ret_sel < 0){
            fprintf(stderr, "Erreur du select dans le fils\n");
            return 1;
         }
         else if(ret_sel == 0){
            fprintf(stderr, "Je suius fatigué d'attendre dans le fils bytesrecv (%ld) byteswrite(%ld)\n", bytesrecv, byteswrite);
            timeout++;
            if(timeout >= 3)
                break;
            sleep(10);
         }
         else
         {
            timeout = 0;
            if (FD_ISSET(pfd[0], &readset)){
               if(!bytesrecved){
                  bytesrecved = read(pfd[0],buffer, BUFFSIZE);
                  if(with_pselect)
                     kill(ppid, SIGALRM);
                  if(bytesrecved < 0){
                     fprintf(stderr, "On a un petit problème lors du read dans le fils\n");
                  }
                  else if (bytesrecved == 0){
                     close(pfd[0]);
                     fprintf(stderr, "end of child\n");
                     break;
                  }
                  else{
                     bytesrecv +=bytesrecved;
                     should_write = 1;
                     offset = 0;
                  }
               }
               //memset(buffer, 0, BUFFSIZE);
            }
            if(FD_ISSET(1, &writeset)){
               if(should_write){
                  int m = write(1, buffer+offset, bytesrecved);
                  if(m < 0)
                     fprintf(stderr, "un problème dans le fils %s", strerror(errno));
                  else {  
                     byteswrite +=m;
                     offset +=m;
                     bytesrecved -= m;
                     if(!bytesrecved){
                        should_write = 0;
                        offset = 0;
                     } 
                  }
                  
               }
            }                
         }
         FD_CLR(pfd[0], &readset);
         FD_CLR(1, &writeset);      
         
      }//Fin while du select dans le fils
      close(pfd[0]);
      exit(0);
   }//FIn du if (si on est dans le fils)
   
    else
  {
     long bytessent = 0;
     long bytesread = 0;
     if(with_pselect)
        if (signal(SIGALRM, handle_alarm) == SIG_ERR)
          fprintf(stderr, "Signal %d non capture\n", SIGALRM);

     while(1){
        fd_max = 0;
        FD_SET(0, &readset);
        FD_SET(pfd[1], &writeset);
        if (pfd[1] > fd_max)
           fd_max = pfd[1];
        
        if (with_time)
           ret_sel = select(fd_max+1, &readset, &writeset, NULL, &out_time);
        else if(with_pselect){
           sigaddset(&pselect_set, SIGALRM);
           ret_sel = pselect(fd_max+1, &readset, &writeset, NULL, NULL, &pselect_set);
        }
        else 
           ret_sel = select(fd_max+1, &readset, &writeset, NULL, NULL);
        
        if (ret_sel < 0){
            fprintf(stderr,"Une erreur est survenue! dans le select du père\n");
            break;
        }
        else if (ret_sel == 0){
            fprintf(stderr,"Temps expiré, rien à lire, rien à écrire dans le père!\n");
            break;
        }
        else{
           if(FD_ISSET(0, &readset)){
              if(!bytestosend){
                 ret = read(0, buffer, BUFFSIZE);
                 if(ret < 0)
                    fprintf(stderr, "Il ya eu un problème lors de la lecture\n");
                 else if(ret>0){
                    should_write = 1;
                    bytestosend += ret;
                    bytesread +=ret;
                 }
                 else {
                  fprintf(stderr, "end of father\n");
                  break;
                 }
              }
              else{
                 if(offset)
                     fprintf(stderr, "Peut on comblé le vide (%d)(%d)\n", bytestosend, offset);
              }
               
            }

            if (FD_ISSET(pfd[1], &writeset))
            {
               
               //scanf("%s", buffer);
               if (should_write){
                  ret = write(pfd[1], buffer+offset, bytestosend);
                  if(ret < 0){
                     fprintf(stderr, "il ya eu un problème lors de l'écriture %d\n", offset);
                     //break;
                  }
                  bytessent += ret;
                  bytestosend -= ret; 
                  offset +=  ret;
                  
                  if(bytestosend){
                     fprintf(stderr, "on a pas pu tout envoyer %d\n", bytestosend);
                  }else{
                    should_write = 0;
                    offset = 0;
                    //fprintf(stderr, "on a tout envoyé %d\n", bytestosend);
                  //memset(buffer, 0, BUFFSIZE); 
                  }              
               }
            }
            
         }
         
         FD_CLR(0, &readset);
         FD_CLR(pfd[1], &writeset);
      }
   int status;
   fprintf(stderr, "waiting child bytesread (%ld), bytessent (%ld)\n", bytesread, bytessent);
   int pid2 = wait(&status);
  }
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
