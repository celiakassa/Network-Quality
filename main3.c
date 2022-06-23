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
int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
int start_workers2(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
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
  pid_t pid1, pid2, wpid;
  int status = 0;
  //printf("Start measurement \n\n");
   //création des mémoires partagées pour savoir si la saturation est atteinte et si le calcul est terminé
  int shmid[6],resp1 = 2,ret,*downsatured,*upsatured,resp2,*rpmfinished,resp3,resp4,resp5,*syncdata1,*syncdata2,*syncdata3;
   
  
  
  //SaturationResult=saturate(configurls.LargeUrl,configurls.UploadUrl);
  printf("Un père sans fils\n");
  /*Creation de processus fils pour download*/
  if ((pid1 = fork()) == -1){
      printf("Echec de la création du processus filslarge\n");
       exit(1);
  }
  if (pid1 == 0){// fils du download   
     shmid[0] = init(getppid(), PROJ_ID);
     ret = mem_attach(shmid[0], (void**)&downsatured);
     if(ret == -1){
     		printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
     }
     *downsatured = *downsatured | 1 << 0;
     shmid[2] = init(getppid(),PROJ_ID+2);
     ret = mem_attach(shmid[2],(void**)&rpmfinished);
     if(ret == -1){
        printf("Failed to configure shared memory %d\n", errno);
        return SHM_ERR;
     }
     printf("Qui est mon papa %d?? Moi même je suis %d: %d rpm? %d\n\n", getppid(), getpid(), *downsatured , *rpmfinished);   
  }
  else{
     printf("Tototototototo\n\n");
     if ((pid2 = fork()) == -1){
        printf("Echec de la création du processus filsup\n");
      
     }

     if (pid2 == 0){// fils du upload
        shmid[1] = init(getppid(),PROJ_ID+1);
        ret = mem_attach(shmid[1],(void**)&upsatured);
        if(ret == -1){
           printf("Failed to configure shared memory %d\n", errno);
           return SHM_ERR;
        }
        *upsatured = *upsatured | 1 << 0;  
        
        shmid[2] = init(getppid(),PROJ_ID+2);
        ret = mem_attach(shmid[2],(void**)&rpmfinished);
        if(ret == -1){
           printf("Failed to configure shared memory %d\n", errno);
           return SHM_ERR;
        }
        printf("Qui est mon papa %d?? Moi même je suis %d %d rpm? %d\n\n", getppid(), getpid(), *upsatured, *rpmfinished);
     }
     else{
        printf("le père attends ses fils\n");
        shmid[0] = init(getpid(), PROJ_ID);
        ret = mem_attach(shmid[0], (void**)&downsatured);
        if(ret == -1){
     	   printf("Failed to configure shared memory %d\n", errno);
     	   return SHM_ERR;  
        }   
  	*downsatured = 0;
  	
        shmid[1] = init(getpid(), PROJ_ID+1);
        ret = mem_attach(shmid[1], (void**)&upsatured);
        if(ret == -1){
     	   printf("Failed to configure shared memory %d\n", errno);
     	   return SHM_ERR;  
        }
        *upsatured = 0;
        
        shmid[2] = init(getpid(), PROJ_ID+2);
        ret = mem_attach(shmid[2], (void**)&rpmfinished);
        if(ret == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
        }
        *rpmfinished = 1000;
         
        while ((wpid = wait(&status)) > 0){
           printf("le fils %d s'est terminé %d %d\n", wpid, *downsatured, *upsatured); 
        }
     }
  }
  return 0;
}

 
