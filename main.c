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
#include "task.h"

int handle_sign = 0;

void handle_alarm(int number){
  if(number == SIGALRM)
     handle_sign = 1;	        
}
int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv[]);
int main(int argc, char *argv[]){
  if(argc!=3){
    printf("Usage %s <url> <resources>\n", argv[0]);
    return ARG_ERR;
  }
  //pid_t pid[WORKERS], pid2[WORKERS];
  //long *recv_bytes[WORKERS], *recv_bytes2[WORKERS];
  //int i;
  //int shmid[WORKERS+1], shmid2[WORKERS+1];
  Tasks t;
  t = init_tasks_list();
  //t = addTask(pid, shmid, recv_bytes, t);
  t = addTask(t);
  if (signal(SIGALRM, handle_alarm) == SIG_ERR){
     printf("Failed to onfigure signal %d \n", SIGALRM);
     return SIGNAL_ERR;
  }
  
  
  start_workers(t->pid, t->shmid, t->recv_bytes, argv);
  //t = addTask(pid2, shmid2, recv_bytes2, t);
  t = addTask(t);
  start_workers(t->pid, t->shmid, t->recv_bytes, argv);
  
  t = addTask(t);
  start_workers(t->pid, t->shmid, t->recv_bytes, argv);
  alarm(1);
  
  while(1){
     if(handle_sign){
        /*for(i = 0; i < WORKERS; i++)
        printf("%ld ", *recv_bytes[i]);*/
        get_recv_bytes(t);
        handle_sign = 0;
     }
     printf("\n");
     alarm(1);
     pause();
  }
  
 /* for(i = 0; i < 4; i++){
     int status;
     pid[i] = wait(&status);
  }*/
  
  return 0;
}

int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv[]){
  int *syncdata, ret, i;
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
    ret = mem_attach(shmid[i+1], (void**)(recv_bytes + i));
    if(ret == -1){
       printf("Failed to configure shared memory %d\n", errno);
       return SHM_ERR;  
    }
  }
  return OK;
}
