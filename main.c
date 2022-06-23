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
  pid_t pid1, pid2,wpid;
  int status = 0;
  //printf("Start measurement \n\n");
   //création des mémoires partagées pour savoir si la saturation est atteinte et si le calcul est terminé
  int shmid[6],resp1,resp0,*downsatured,*upsatured,resp2,*rpmfinished,resp3,resp4,resp5,*syncdata1,*syncdata2,*syncdata3;
   
  
  
  //SaturationResult=saturate(configurls.LargeUrl,configurls.UploadUrl);
  printf("Un père sans fils\n");
  /*Creation de processus fils pour download*/
  if ((pid1 = fork()) == -1)
   {
      printf("Echec de la création du processus filslarge\n");
       exit(1);
   }
  if (pid1 == 0)// fils du download
   {   
      printf("Dans le fils download\n");
      int shmid[3],resp0,resp1,resp2,resp3,*downsatured,*rpmfinished,*syncdata1,*syncdata3;//1 pour dire au père qu'il est saturé, un autre par lequel le père l'informe de libérer ses fils et le dernier par lequel il les informe
      
      //create shared memory
      shmid[0] = init(getppid(), PROJ_ID);
      resp0 = mem_attach(shmid[0], (void**)&syncdata1);
      if(resp0 == -1){
     		printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
      }
      *syncdata1 = *syncdata1 | 1 << 0;
      while(*syncdata1!=RDV1);
     
     shmid[1] = init(getppid(),PROJ_ID);
     resp1 = mem_attach(shmid[1],(void**)&downsatured);
     if(resp1 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }  
     printf("downsatured dans down %d\n",*downsatured);
     
     shmid[2] = init(getppid(), PROJ_ID);
     resp2 = mem_attach(shmid[2], (void**)&syncdata3);
     if(resp2 == -1){
     		printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
      }
       printf("bloqué dans down \n"); 
      *syncdata3 = *syncdata3 | 1 << 0;
      while(*syncdata3!=RDV2);
      
     shmid[3] = init(getppid(),PROJ_ID);
     resp3 = mem_attach(shmid[3],(void**)&rpmfinished);
     if(resp3 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }
     printf("rpmfinished dans down %d\n",*rpmfinished);
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
       
     printf("rpmfinished dans down niveau 2 %d\n",*rpmfinished);
   	 
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
     
    while(t!=NULL)
         t = delTask(t);
     // exit(1);
   }
    if ((pid2 = fork()) == -1)
   {
      printf("Echec de la création du processus filsup\n");
      
   }

   if (pid2 == 0)// fils du upload
   {
     printf("Dans le fils upload\n");
     int shmid[3],resp1,resp2,resp0,*upsatured,*rpmfinished,*syncdata2,*syncdata3;//1 pour dire au père qu'il est saturé, un autre par lequel le père l'informe de libérer ses fils et le dernier par lequel il les informe
      
      //create shared memory
      shmid[0] = init(getppid(), PROJ_ID);
      resp0 = mem_attach(shmid[0], (void**)&syncdata2);
      if(resp0 == -1){
     		printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
      }
      printf("Dans le fils upload etape1\n");
      *syncdata2 = *syncdata2 | 1 << 1;
      while(*syncdata2!=RDV1);
     
     shmid[1] = init(getppid(),PROJ_ID);
     resp1 = mem_attach(shmid[1],(void**)&upsatured);
     if(resp1 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }  
     printf("upsatured dans up %d\n",*upsatured);
     
     shmid[2] = init(getppid(), PROJ_ID);
     resp2 = mem_attach(shmid[2], (void**)&syncdata3);
     if(resp2 == -1){
     		printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
      }
      printf("bloqué dans up \n"); 
     *syncdata3 = *syncdata3 | 1 << 1;
     while(*syncdata3!=RDV2);
      
     shmid[3] = init(getppid(),PROJ_ID);
     resp3 = mem_attach(shmid[3],(void**)&rpmfinished);
     if(resp3 == -1){
        printf("Failed to configure shared memory %d\n", errno);
     //return SHM_ERR;
     }
     printf("rpmfinished dans up %d\n",*rpmfinished);
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
   printf("le père avec des fils\n"); 
   shmid[0] = init(getpid(), PROJ_ID);
  resp0 = mem_attach(shmid[0], (void**)&syncdata1);
  if(resp0 == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
  }   
  *syncdata1 = 0;
  
  shmid[1] = init(getpid(), PROJ_ID);
   resp1 = mem_attach(shmid[1], (void**)&syncdata2);
   if(resp1 == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
  }   
  *syncdata2 = 0;
  
  shmid[2] = init(getpid(), PROJ_ID);
   resp2 = mem_attach(shmid[2], (void**)&syncdata3);
   if(resp2 == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
  }   
  *syncdata3 = 0;
  while(*syncdata1!=RDV1);
  shmid[3] = init(getpid(), PROJ_ID);
   resp3 = mem_attach(shmid[3], (void**)&downsatured);
   if(resp3 == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
  }   
  *downsatured = 0;
 
  while(*syncdata2!=RDV1);
   printf("le père est bloqué ici\n"); 
  shmid[4] = init(getpid(), PROJ_ID);
  resp4 = mem_attach(shmid[1], (void**)&upsatured);
   if(resp4 == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
  }
  *upsatured = 0;
  printf("le père est bloqué ici\n"); 
  while(*syncdata3!=RDV2);
   shmid[5] = init(getpid(), PROJ_ID);
   resp5 = mem_attach(shmid[5], (void**)&rpmfinished);
   if(resp5 == -1){
     	printf("Failed to configure shared memory %d\n", errno);
     	return SHM_ERR;  
  }
  *rpmfinished = 0;
  /*while(*downsatured && *upsatured){
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
      tab[i].duration = dns("www.uclouvain.be");   
    }
  
    for (i = i; i < 10; i++){
      tab[i].type = TCP;
      tab[i].duration=tcp("www.uclouvain.be");  
    }
    for (i = i; i < 15; i++){
      tab[i].type = TLS;
      tab[i].duration=tls("www.uclouvain.be:https");
    }
    for (i = i; i < 20; i++){
     tab[i].type = DWN;
     tab[i].duration=down(param1,param2);
    }  
    for (i = i; i < 25; i++){
      tab[i].type = UP;
      tab[i].duration=up(param1,param2);
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
    *rpmfinished = 1;
    break;
    }
    
    //il faut une mémoire partagée ici pour dire à la fonction saturate de libérer ses fils*/
  	while ((wpid = wait(&status)) > 0);
  }
  return 0;
}

 
