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

//#include "saturateup.h"
int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
int start_workers2(pid_t *pid, int *shmid, long **recv_bytes, char *argv1[]);
int main(int argc, char *argv[]){
   if(argc!=3){
    printf("Usage %s <url> <resources>\n", argv[0]);
    return ARG_ERR;
   }
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
      //*downsatured = *downsatured | 1 << 0;
      shmid[2] = init(getppid(),PROJ_ID+2);
      ret = mem_attach(shmid[2],(void**)&rpmfinished);
      if(ret == -1){
        printf("Failed to configure shared memory %d\n", errno);
        return SHM_ERR;
      }
      printf("Qui est mon papa %d?? Moi même je suis %d: %d rpm? %d\n\n", getppid(), getpid(), *downsatured , *rpmfinished);   
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
                     //printf("down workers  %d\n", workers);  
                     //printf("downsatured  %d\n", *downsatured);     
                     //printf("rpmfinished  %d\n", *rpmfinished);    
                     *downsatured = 1;
                     //printf("downsatured 2 %d\n", *downsatured);       
                     while(!*rpmfinished);     
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
      while(t!=NULL)
               t = delTask(t);
      printf("le while(1) de down est fini\n");
   }
   else{
     // printf("Tototototototo\n\n");
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
         //*upsatured = *upsatured | 1 << 0;  
         shmid[2] = init(getppid(),PROJ_ID+2);
         ret = mem_attach(shmid[2],(void**)&rpmfinished);
         if(ret == -1){
           printf("Failed to configure shared memory %d\n", errno);
           return SHM_ERR;
         }
         printf("Qui est mon papa %d?? Moi même je suis %d %d rpm? %d\n\n", getppid(), getpid(), *upsatured, *rpmfinished);
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
                        //printf("up workers  %d\n", workers);
                        //printf("upsatured  %d\n", *upsatured); 
                        //printf("rpmfinished  %d\n", *rpmfinished);    
                        *upsatured = 1;  
                        //printf("upsatured 2 %d\n", *upsatured);    
                        while(!*rpmfinished);                                      
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
         while(t!=NULL)
            t = delTask(t);

         printf("le while(1) de up est fini\n");
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
         printf("upsatured 2 %d\n", *upsatured); 
           printf("downsatured 2 %d\n", *downsatured);  
         while (!*downsatured || !*upsatured);
            printf("start mesurement\n");
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
            for (i = 0; i < 5; i++){
            tab[i].type = DNS;
            tab[i].duration = dns("monitor.uac.bj");   
            }
            printf("Debugging 1 \n"); 
            for (i = i; i < 10; i++){
               tab[i].type = TCP;
               tab[i].duration=tcp("www.uclouvain.be");  
            }
            printf("Debugging 2 \n"); 

            for (i = i; i < 15; i++){
               tab[i].type = TLS;
               tab[i].duration=tls("monitor.uac.bj:4449");
            }
            printf("Debugging 3 \n"); 
            for (i = i; i < 20; i++){
               tab[i].type = DWN;
               tab[i].duration=down(param1,param2);
            }  
            printf("Debugging 4 \n"); 
             for (i = i; i < 25; i++){
               tab[i].type = UP;
               tab[i].duration=up(param1,param2);
            }  
            printf("Debugging 5 \n"); 
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
            printf("dnsmoy %ld\n",dnsmoy  ); 
            printf("tcpmoy %ld\n",tcpmoy  ); 
            printf("tlsmoy %ld\n",tlsmoy  ); 
            printf("dwnmoy %ld\n",dwnmoy  ); 
            printf("upmoy %ld\n",upmoy  ); 
            dnsmoy=dnsmoy/5L;
            tcpmoy=tcpmoy/5L;
            tlsmoy=tlsmoy/5L;
            dwnmoy=dwnmoy/5L;
            upmoy=upmoy/5L;
             printf("dnsmoy 2 %ld\n",dnsmoy  ); 
            printf("tcpmoy 2 %ld\n",tcpmoy  ); 
            printf("tlsmoy  2 %ld\n",tlsmoy  ); 
            printf("dwnmoy 2 %ld\n",dwnmoy  ); 
            printf("upmoy 2 %ld\n",upmoy  ); 
            long total_s =  (dnsmoy+tcpmoy+tlsmoy+dwnmoy+upmoy)/1000000000L;
            rpm = 60L/total_s;
            printf("RPM: %ld", rpm);
            *rpmfinished = 1;
            
         while ((wpid = wait(&status)) > 0){
            printf("le fils %d s'est terminé %d %d\n", wpid, *downsatured, *upsatured); 
         }
      }
   }
  return 0;
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
