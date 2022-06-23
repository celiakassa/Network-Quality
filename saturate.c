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

int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
int start_workers2(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
SaturateResult saturate(char *largeurl,char *uploadurl){
   
   pid_t pid1, pid2,wpid;
   int status = 0;
   SaturateResult SaturationResult;  
   SaturationResult.oksaturation=0; 
   
   /*Creation de processus fils pour download*/
   if ((pid1 = fork()) == -1)
   {
      printf("Echec de la création du processus filslarge\n");
      SaturationResult.error=1;
     	return SaturationResult; 
     //	exit(1);      
   }
   if (pid1 == 0)// fils du download
   {   
    printf("Dans le fils download\n");
   int shmid[3],resp1,resp2,resp3,*downsatured,*rpmfinished;//1 pour dire au père qu'il est saturé, un autre par lequel le père l'informe de libérer ses fils et le dernier par lequel il les informe
      
      //create shared memory
  
     shmid[0] = init(getppid(),PROJ_ID);
     resp1 = mem_attach(shmid[0],(void**)&downsatured);
     if(resp1 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }
     
     shmid[1] = init(getppid(),PROJ_ID);
     resp2 = mem_attach(shmid[0],(void**)&rpmfinished);
     if(resp2 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }
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
      long prev_mva = -1, cur_mva;
      int workers = 0;
      Tasks t;
      t = init_tasks_list();
      t = addTask(t);
      if (signal(SIGALRM, handle_signal) == SIG_ERR){
         printf("Failed to onfigure signal %d \n", SIGALRM);
        //return SIGNAL_ERR;
      }  
      if (signal(SIGINT, handle_signal) == SIG_ERR){
        printf("Failed to onfigure signal %d \n", SIGINT);
        //return SIGNAL_ERR;
      }
      char *argv1[10];
      argv1[1]="https://monitor.uac.bj:4449";
      argv1[2]="large";  	
      start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
      workers+=4;
      alarm(1);
       
   	printf("rpmfinished dans down %d\n",*rpmfinished);
   	 
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
                time = 0; 
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
            if((time >= 4) /*&& !saturation*/){
                t = addTask(t);
                start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                time = 0; 
                workers+=4;
            }
        }else{
              if(!saturation && !mva_change){
                 t = addTask(t);
                 start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                 time = 0; 
                 workers+=4;
                 saturation = 1;
              }else{            
                    if(saturation && mva_change){
                       t = addTask(t);
                       start_workers(t->pid, t->shmid, t->recv_bytes, argv1);
                       time = 0; 
                       workers+=4;
                       mva_change = 0;                   
                    }
                    if(time >= 4 /*&& !mva_change*/){                        
                       stable_saturation = 1;  
                       printf("down workers  %d\n", workers);   
                       *downsatured = 1;    
                       while(*rpmfinished !=1);     
                       break;
                    }
              }
        }       
        prev_mva = cur_mva;
        handle_alrm = 0;
        alarm(1);
     }
     if(handle_int){
        while(t!=NULL)
      	  t = delTask(t);
      }
     pause();
    }
    /*$if(*rpmfinished == 1){
    	printf("up workers finished\n"); 
      while(t!=NULL)
         t = delTask(t);
      exit(1);
    }*/
    while(t!=NULL)
         t = delTask(t);
     // exit(1);
   
   }
     /*Creation de processus fils pour upload*/
   if ((pid2 = fork()) == -1)
   {
      printf("Echec de la création du processus filsup\n");
      SaturationResult.error=1;
      exit(1);
   }

   if (pid2 == 0)// fils du upload
   {
     printf("Dans le fils upload\n");
      int shmid[3],resp1,resp2,*upsatured,*rpmfinished;//1 pour dire au père qu'il est saturé, un autre par lequel le père l'informe de libérer ses fils et le dernier par lequel il les informe
      
      //create shared memory
  
     shmid[0] = init(getppid(),PROJ_ID);
     resp1 = mem_attach(shmid[0],(void**)&upsatured);
     if(resp1 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }
      shmid[1] = init(getppid(),PROJ_ID);
     resp2 = mem_attach(shmid[0],(void**)&rpmfinished);
     if(resp2 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }
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
       long prev_mva = -1, cur_mva;
       int workers = 0;
       Tasks t;
       t = init_tasks_list();
       t = addTask(t);
       if (signal(SIGALRM, handle_signal) == SIG_ERR){
         printf("Failed to onfigure signal %d \n", SIGALRM);
        // return SIGNAL_ERR;
       } 
       if (signal(SIGINT, handle_signal) == SIG_ERR){
          printf("Failed to onfigure signal %d \n", SIGINT);
          //return SIGNAL_ERR;
       }
      char *argv2[10];
      argv2[1]="https://monitor.uac.bj:4449";
      argv2[2]="slurp";
      start_workers2(t->pid, t->shmid, t->recv_bytes, argv2);
      workers+=4;
      alarm(1);
        printf("rpmfinished dans Up %d\n",*rpmfinished);
   	 
      while(1){//boucle de saturation
         
         if(handle_alrm){     
         get_recv_bytes(t);
         cur_mva = compute_moving_avg();
         long tmp_mva = (long) 1.05 * prev_mva;
         if(cur_mva > tmp_mva){
             if(saturation && !mva_change){
                mva_change = 1;
                t = addTask(t);
                start_workers2(t->pid, t->shmid, t->recv_bytes, argv2);
                time = 0; 
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
            if((time >= 4) /*&& !saturation*/){
                t = addTask(t);
                start_workers2(t->pid, t->shmid, t->recv_bytes, argv2);
                time = 0; 
                workers+=4;
            }
        }else{
              if(!saturation && !mva_change){
                 t = addTask(t);
                 start_workers2(t->pid, t->shmid, t->recv_bytes, argv2);
                 time = 0; 
                 workers+=4;
                 saturation = 1;
              }else{
              
                    if(saturation && mva_change){
                       t = addTask(t);
                       start_workers2(t->pid, t->shmid, t->recv_bytes, argv2);
                       time = 0; 
                       workers+=4;
                       mva_change = 0;                   
                    }
                    if(time >= 4 /*&& !mva_change*/){                        
                       stable_saturation = 1; 
                        printf("up workers  %d\n", workers);   
                       *upsatured = 1;     
                        while(*rpmfinished !=1);                                      
                       break;
                    }
              }
        }        
        prev_mva = cur_mva;
        handle_alrm = 0;
        alarm(1);
     }
      if(handle_int){
         while(t!=NULL)
           t = delTask(t);
     }
     pause();
    }
    /*if(*rpmfinished){
    	printf("up workers finished\n"); 
    
    }*/
    
      while(t!=NULL)
         t = delTask(t);
     // exit(1);
   }
   else{
        
 	printf("Dans le fils père\n");
   
   		
         //return SaturationResult;
   }
 
}

int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]){
  int *syncdata, ret, i;
  for(i = 0; i < WORKERS; i++){
     char *id = (char*)malloc(ID_SIZE);
     sprintf(id, "%d", i);
     char *const worker_argv[] = {WORKER_TASK, argv1[1], argv1[2], id, NULL};
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

int start_workers2(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]){
  int *syncdata, ret, i;
  for(i = 0; i < WORKERS; i++){
     char *id = (char*)malloc(ID_SIZE);
     sprintf(id, "%d", i);
     char *const worker_argv[] = {WORKER_TASK2, argv1[1], argv1[2], id, NULL};
     pid[i] = fork();
     if(pid[i] < 0){     
        fprintf(stderr, "Erreur de création du processus (%d)\n", errno);
        return 1;
    }   
    if(pid[i] == 0){
      execve(WORKER_TASK2, worker_argv, NULL);
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
