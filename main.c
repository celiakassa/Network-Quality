#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "shm.h"
#include "task.h"
#include "test.h"
#include "config.h"
#include "saturate.h"
//#include "saturateup.h"
 
int main(int argc, char *argv[]){
  if(argc!=3){
    printf("Usage %s <url> <resources>\n", argv[0]);
    return ARG_ERR;
  }
   SaturateResult SaturationResult;
  printf("start ping server\n");
  Configurls configurls = configuration(argv);
  printf("server configuration:\n");
  printf("SmallUrl :%s\n",configurls.SmallUrl);
  printf("LargeUrl :%s\n",configurls.LargeUrl);
  printf("UploadUrl: %s\n",configurls.UploadUrl);
  printf("start saturation\n");
    
  //printf("Start measurement \n\n");
 
  SaturationResult=saturate(configurls.LargeUrl,configurls.UploadUrl);
   // printf("SaturateDown Results:\n");
  //  printf("Download Flows :%d\n",SaturationResult.flows);
     //SaturateUpResult SaturateResultUp=saturateup(configurls.UploadUrl);
   //printf("SaturateUP Results:\n");
  //  printf("Upload Flows :%d\n",SaturateResultUp.flows);
  int i;
  char mes[1];

  long dnsmoy = 0L;
  long tcpmoy = 0L;
  long tlsmoy = 0L;
  long dwnmoy = 0L;
  long upmoy = 0L;
  long rpm;
  struct mesure tab[25];
    
  char *param1= argv[1];
  char *param2=argv[2];
//  printf("Start  %s\n\n", param1);
  //printf("Start measurement %s\n\n", param2);
  for (i = 0; i < 5; i++){
    tab[i].type = DNS;
    tab[i].duration = dns("www.uclouvain.be");
   // printf("dns");
  }
  
  for (i = i; i < 10; i++){
    tab[i].type = TCP;
    tab[i].duration=tcp("www.uclouvain.be");
    // printf("tcp");
  }
  for (i = i; i < 15; i++){
    tab[i].type = TLS;
    tab[i].duration=tls("www.uclouvain.be:https");
      //printf("tls");
  }
  for (i = i; i < 20; i++){
    tab[i].type = DWN;
    tab[i].duration=down(param1,param2);
     // printf("down");
  }
  
  for (i = i; i < 25; i++){
    tab[i].type = UP;
    tab[i].duration=up(param1,param2);
  //  printf("upload");
  }
  
  for (i = 0; i < 25; i++){
    if( tab[i].type == DNS )
    	dnsmoy+=tab[i].duration;
    if( tab[i].type == TCP)
    	tcpmoy+=tab[i].duration;  
    if( tab[i].type == TLS)
    	tlsmoy+=tab[i].duration;  	
    if( tab[i].type == DWN)
    	dwnmoy+=tab[i].duration;  
    if( tab[i].type == UP)
    	upmoy+=tab[i].duration;  
  }
  
  dnsmoy=dnsmoy/5L;
  tcpmoy=tcpmoy/5L;
  tlsmoy=tlsmoy/5L;
  dwnmoy=dwnmoy/5L;
  upmoy=upmoy/5L;
  long total_s =  (dnsmoy+tcpmoy+tlsmoy+dwnmoy+upmoy)/1000000000L;
  rpm = 60L/total_s;
  printf("RPM: %ld", rpm);
  
 
  return 0;
}

 
