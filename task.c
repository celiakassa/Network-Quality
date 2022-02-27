#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "task.h"
#include "rpm.h"
#include "shm.h"

Instan_aggregate instan_agg;

Tasks init_tasks_list(void){
   Tasks headT = (Tasks) malloc(sizeof(struct task_node));
   headT->next = NULL;
   headT->flag = -1;
   instan_agg = init_instan_list();
   return headT;
}

//Tasks addTask(pid_t *pid, int *shmid, long **recv_bytes, Tasks headT){
Tasks addTask(Tasks headT){
   if(headT->flag == -1){
      headT->flag = 0;
      headT->pid = (pid_t*) malloc(WORKERS*sizeof(pid_t));
      headT->shmid = (int*) malloc((WORKERS+1)*sizeof(int));
      headT->recv_bytes = (long**)malloc(WORKERS*sizeof(long));
      headT->prev_goodput = (long*) malloc((WORKERS)*sizeof(long));
      memset(headT->prev_goodput, 0, (WORKERS)*sizeof(long));
      return headT;
   }
   Tasks n = (Tasks) malloc(sizeof(struct task_node));
   n->flag = 0;
   n->pid = (pid_t*) malloc(WORKERS*sizeof(pid_t));
   n->shmid = (int*) malloc((WORKERS+1)*sizeof(int));
   n->recv_bytes = (long**)malloc(WORKERS*sizeof(long));
   n->prev_goodput = (long*) malloc((WORKERS)*sizeof(long));
   memset(n->prev_goodput, 0, (WORKERS)*sizeof(long));
   n->next = headT;
   headT = n;
   return headT;
}

Tasks delTask(Tasks headT){
  int i;
  if(headT == NULL)
      return headT;
   Tasks n;
   Tasks next = headT->next;
   n = headT;
   headT = next;
   for(i = 0; i < WORKERS; i++){
     if(kill(n->pid[i], SIGINT) == -1)
        fprintf(stderr, "Failed to send SIGINT to process %d", n->pid[i]);
     if(mem_detach((void **)&n->recv_bytes[i]) == -1)
        fprintf(stderr, "Failed to detach from shared memory\n");
   }
   free(n->pid);
   free(n->shmid);
   free(n->recv_bytes);
   free(n->prev_goodput);
   free(n);
   return headT;
}

void get_recv_bytes(Tasks t){
    int i;
    if(t->flag == -1)
      return;
    Tasks next = t->next;
    long goodput = 0;
    long lrecv_bytes;
    for(i = 0; i < WORKERS; i++){
        lrecv_bytes = *(t->recv_bytes)[i];
        goodput+=lrecv_bytes - t->prev_goodput[i];
        t->prev_goodput[i] = lrecv_bytes;
    }
    while(next!=NULL){
       for(i = 0; i < WORKERS; i++){
          lrecv_bytes = *(next->recv_bytes)[i];
          goodput+=lrecv_bytes - next->prev_goodput[i];
          next->prev_goodput[i] = lrecv_bytes;
       }
       next = next->next;
    }
    instan_agg = addAggregate(goodput, instan_agg);
}

long compute_moving_avg(void){
   return get_prev_goodput(instan_agg);
}
