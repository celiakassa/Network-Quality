#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include "shm.h"
#include "workerh2.h"
#include "task.h"
#include "test.h"
#include "config.h"
#include "saturate.h"

void handle_signal(int number);
int handle_alrm = 0;
int handle_int = 0;
int time1 = 0;
int saturation = 0;
int mva_change = 0;
int duration = 0;
int stable_saturation = 0;
void handle_signal(int number){
  if(number == SIGALRM){
     handle_alrm = 1;	
     time1++;
     duration++;
  }
  if((number == SIGINT)){
     handle_int = 1;
  }        
}
int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
SaturateResult saturate(char *largeurl,char *uploadurl){
   
   pid_t pid1, pid2,wpid;
  int status = 0;

   SaturateResult SaturationResult;
    
   //printf("LargeUrl :%s\n",largeurl);
   //printf("UploadUrl: %s\n",uploadurl);
    
   /*Creation de processus fils pour download*/
   if ((pid1 = fork()) == -1)
   {
      printf("Echec de la création du processus filslarge\n");
      SaturationResult.error=1;
     	return SaturationResult; 
     	exit(1);      
   }

   if (pid1 == 0)
   {   
      long prev_mva = -1, cur_mva;
      int workers = 0;
      Tasks t;
      t = init_tasks_list();
      t = addTask(t);
      if (signal(SIGALRM, handle_signal) == SIG_ERR){
        printf("Failed to onfigure signal %d \n", SIGALRM);
        SaturationResult.error=1;
        return SaturationResult;
       //return SIGNAL_ERR;
      }
  
      if (signal(SIGINT, handle_signal) == SIG_ERR){
        printf("Failed to onfigure signal %d \n", SIGINT);
        SaturationResult.error=1;
      	return SaturationResult;
       //return SIGNAL_ERR;
      }
    
      char *argv1[10];
      argv1[1]="https://monitor.uac.bj:4449";
      argv1[2]="large";
   	
      start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
      workers+=4;
      alarm(1);
     
      while(1){
         
         if(handle_alrm){
         
         get_recv_bytes(t);
         cur_mva = compute_moving_avg();
         long tmp_mva = (long) 1.05 * prev_mva;
         if(cur_mva > tmp_mva){
             if(saturation && !mva_change){
                mva_change = 1;
                t = addTask(t);
                start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                time1 = 0; 
                workers+=4;
                saturation = 0;
             }
             /*if(saturation && mva_change){
              printf("timantB\n");
              t = addTask(t);
              start_workers(t->pid, t->shmid, t->recv_bytes, argv);
              tim = 0; 
              workers+=4;
            }*/
            if((time1 >= 4) /*&& !saturation*/){
                t = addTask(t);
                start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                time1 = 0; 
                workers+=4;
            }
        }else{
              if(!saturation && !mva_change){
                 t = addTask(t);
                 start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                 time1 = 0; 
                 workers+=4;
                 saturation = 1;
              }else{
              
                    if(saturation && mva_change){
                       t = addTask(t);
                       start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                       time1 = 0; 
                       workers+=4;
                       mva_change = 0;                   
                    }
                    if(time1 >= 4 /*&& !mva_change*/){
                        
                       stable_saturation = 1;                  
                       SaturationResult.flows=workers;
                       //SaturationResult.rate=&t->recv_bytes;
                       //printf("Saturationdown Result %d \n", SaturationResult.flows);
                        //printf("Saturationdown rates %ln \n", *(t->recv_bytes));
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
   clean:

    while(t!=NULL)
      t = delTask(t);
     
   exit(0) ;
   }
   
     /*Creation de processus fils pour upload*/
   if ((pid2 = fork()) == -1)
   {
      printf("Echec de la création du processus filsup\n");
      SaturationResult.error=1;
      exit(1);
   }

   if (pid2 == 0)
   {
   	printf("uploadUrl processus sucess  (pid = %d)\n", getpid());
   	 exit(0);
   }
   else{
  	 
        //wpid = wait(&status);
         while ((wpid = wait(&status)) > 0)
   	 {
     	 	printf("Exit status of %d was %d (%s)\n", (int)wpid, status, (status > 0) ? "accept" : "reject");
   	 }
         return SaturationResult;
   }
 
}

 int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]){
  int *syncdata, ret, i;
  for(i = 0; i < WORKERS; i++){
     char *id = (char*)malloc(ID_SIZE);
     sprintf(id, "%d", i);
     char *const worker_argv[] = {WORKER_TASK, argv1[1], argv1[2], id, NULL};
    //printf("%s\n",worker_argv[3]);    
     pid[i] = fork();
     if(pid[i] < 0){     
        fprintf(stderr, "Erreur de création du processus (%d)\n", errno);
        return 1;
    }   
    if(pid[i] == 0){
      //printf("quelque chose de nouveaux\n");
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
