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
#include "test.h"

int handle_alrm = 0;
int handle_int = 0;
int time = 0;
int saturation = 0;
int mva_change = 0;
int duration = 0;
int stable_saturation = 0;

void handle_signal(int number){
  if(number == SIGALRM){
     handle_alrm = 1;	
     time++;
     duration++;
  }
  if((number == SIGINT)){
     handle_int = 1;
  }        
}

int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv[]);
int main(int argc, char *argv[]){
  if(argc!=3){
    printf("Usage %s <url> <resources>\n", argv[0]);
    return ARG_ERR;
  }
  long prev_mva = -1, cur_mva;
  int workers = 0;
  Tasks t;
  t = init_tasks_list();
  t = addTask(t);
  if (signal(SIGALRM, handle_signal) == SIG_ERR){
     printf("Failed to onfigure signal %d \n", SIGALRM);
     return SIGNAL_ERR;
  }
  
  if (signal(SIGINT, handle_signal) == SIG_ERR){
     printf("Failed to onfigure signal %d \n", SIGINT);
     return SIGNAL_ERR;
  }
  
  start_workers(t->pid, t->shmid, t->recv_bytes, argv);
  workers+=4;
  alarm(1);
  
  while(1){
     if(handle_alrm){
        get_recv_bytes(t);
        cur_mva = compute_moving_avg();
        if(cur_mva > 1.05 * prev_mva){
           if(saturation && !mva_change){
              mva_change = 1;
              t = addTask(t);
              start_workers(t->pid, t->shmid, t->recv_bytes, argv);
              time = 0; 
              workers+=4;
              //saturation = 0;
           }
           if(saturation && mva_change){
              t = addTask(t);
              start_workers(t->pid, t->shmid, t->recv_bytes, argv);
              time = 0; 
              workers+=4;
           }
           if((time >= 4) /*&& !saturation*/){
              t = addTask(t);
              start_workers(t->pid, t->shmid, t->recv_bytes, argv);
              time = 0; 
              workers+=4;
           }
        }else{
              if(!saturation && !mva_change){
                 t = addTask(t);
                 start_workers(t->pid, t->shmid, t->recv_bytes, argv);
                 time = 0; 
                 workers+=4;
                 saturation = 1;
                 mva_change = 0;
              }else{
              
                    if(saturation && mva_change){
                       t = addTask(t);
                       start_workers(t->pid, t->shmid, t->recv_bytes, argv);
                       time = 0; 
                       workers+=4;
                       mva_change = 0;                   
                    }
                    if(time >= 4 /*&& !mva_change*/){
                       printf("\n\n\nStable Saturation du reseau cur(%ld)  prev(%ld) duration(%d) workers(%d) time(%d)\n\n\n", cur_mva, prev_mva, duration,workers, time);
                       stable_saturation = 1;
                       break;
                    }
              }
         }
        prev_mva = cur_mva;
        handle_alrm = 0;
        alarm(1);
     }
     if(handle_int)
        goto clean;
     pause();
     
  }
  
  int i;
  char mes[1];
  double dnsmoy =0;
  double tcpmoy =0;
  double rpm;
  struct mesure tab[10];
  for (i = 0; i < 5; i++)
  {
    (tab[i].test_type)=atoi("dns");
    tab[i].duration=dns("www.uclouvain.be");
  }
  
  for (i = 0; i < 5; i++)
  {
    (tab[i].test_type)=atoi("tcp");
    tab[i].duration=tcp("www.uclouvain.be");
  }
  
  for (i = 0; i < 10; i++)
  {
    
    if( &(tab[i].test_type) == "dns")
    	dnsmoy+=tab[i].duration;
    	
    if( &(tab[i].test_type) == "tcp")
    	tcpmoy+=tab[i].duration;  
  }
  
  dnsmoy=dnsmoy/5;
  tcpmoy=tcpmoy/5;
  rpm =(dnsmoy+tcpmoy)/(double)60000;
  printf("RPM== %lf;", rpm);
  
clean:

  while(t!=NULL)
     t = delTask(t);
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
  while(*syncdata!=RDV);
  
  for(i = 0; i < WORKERS; i++){
    shmid[i+1] = init(pid[i], PROJ_ID);
    ret = mem_attach(shmid[i+1], (void**)(recv_bytes + i));
    if(ret == -1){
       printf("Failed to configure shared memory %d\n", errno);
       return SHM_ERR;  
    }
  }
  return OK;
}
