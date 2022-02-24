#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include "shm.h"

void handle_alarm(int number){
  fprintf(stderr, "\n%ld a recu le signal %d (%s)\n", 
    (long) getpid(), number, strsignal(number));
  alarm(1);	        
}

int main(int argc, char *argv[]){
  if(argc!=3){
    printf("Usage %s <url> <resources>\n", argv[0]);
    return 1;
  }
  /*char *dest_url = argv[1];
  char *request = argv[2];*/
  pid_t pid[4];
  int *syncdata;
  if (signal(SIGALRM, handle_alarm) == SIG_ERR)
     printf("Signal %d non capture\n", SIGALRM);
  
  for(int i = 0; i < 4; i++){
     char *id = (char*)malloc(2);
     sprintf(id, "%d", i);
     char *const newargv[] = {"./worker", argv[1], argv[2], id, NULL};
     pid[i] = fork();
     if(pid[i] < 0){
        fprintf(stderr, "Erreur de création du processus (%d)\n", errno);
        return 1;
    }
    if(pid[i] == 0){
      //int ret;
      execve("./worker", newargv, NULL);
      perror("execve");
    }
  }
  
  int shmid = init(getpid(), 256);
  int ret = mem_attach(shmid, (void**)&syncdata);
  if(ret != -1)
     *syncdata = 0;
  else
     printf("errno:: %d\n", errno);
  printf("%p\n",(void*) syncdata);
  while(*syncdata!=15);
  
  for(int i = 0; i < 4; i++){
    shmid = init(pid[i], 256);
    printf("shmid %d \n", shmid);
  }
  
  alarm(1);
  
  while(1){
     pause();
  }
  
  for(int i = 0; i < 4; i++){
     int status;
     pid[i] = wait(&status);
  }
  
  return 0;
}
