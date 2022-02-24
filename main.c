#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "shm.h"
#include "worker.h"

int handle_sign = 0;

void handle_alarm(int number){
  if(number == SIGALRM)
     handle_sign = 1;	        
}

int main(int argc, char *argv[]){
  if(argc!=3){
    printf("Usage %s <url> <resources>\n", argv[0]);
    return ARG_ERR;
  }
  pid_t pid[WORKERS];
  int *syncdata;
  long *recv_bytes[WORKERS];
  int ret, i;
  int shmid[WORKERS+1];
  
  if (signal(SIGALRM, handle_alarm) == SIG_ERR){
     printf("Failed to onfigure signal %d \n", SIGALRM);
     return SIGNAL_ERR;
  }
  
  for(i = 0; i < WORKERS; i++){
     char *id = (char*)malloc(ID_SIZE);
     sprintf(id, "%d", i);
     char *const worker_argv[] = {WORKER_TASK, argv[1], argv[2], id, NULL};
     pid[i] = fork();
     if(pid[i] < 0){
        fprintf(stderr, "Erreur de création du processus (%d)\n", errno);
        return 1;
    }
    if(pid[i] == 0){
      //int ret;
      execve(WORKER_TASK, worker_argv, NULL);
      perror("execve");
    }
  }
  
  shmid[0] = init(getpid(), PROJ_ID);
  ret = mem_attach(shmid[0], (void**)&syncdata);
  if(ret == -1){
     printf("Failed to configure shared memory %d\n", errno);
     return SHM_ERR;  
  }
  *syncdata = 0;
  while(*syncdata!=15);
  
  for(i = 0; i < WORKERS; i++){
    shmid[i+1] = init(pid[i], 256);
    ret = mem_attach(shmid[i+1], (void**)&recv_bytes[i]);
    if(ret == -1){
       printf("Failed to configure shared memory %d\n", errno);
       return SHM_ERR;  
    }
  }
  
  alarm(1);
  
  while(1){
     if(handle_sign){
        for(i = 0; i < WORKERS; i++)
        printf("%ld ", *recv_bytes[i]);
        handle_sign = 0;
     }
     printf("\n");
     alarm(1);
     pause();
  }
  
  for(i = 0; i < 4; i++){
     int status;
     pid[i] = wait(&status);
  }
  
  return 0;
}
