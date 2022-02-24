#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include "task.h"

Tasks init_tasks_list(void){
   Tasks headT = (Tasks) malloc(sizeof(struct task_node));
   headT->next = NULL;
   headT->flag = -1;
   return headT;
}

Tasks addTask(pid_t *pid, int *shmid, long **recv_bytes, Tasks headT){
   if(headT->flag == -1){
      headT->flag = 0;
      headT->pid = pid;
      headT->shmid = shmid;
      headT->recv_bytes = recv_bytes;
      return headT;
   }
   Tasks n = (Tasks) malloc(sizeof(struct task_node));
   n->flag = 0;
   n->pid = pid;
   n->shmid = shmid;
   n->recv_bytes = recv_bytes;
   n->next = headT;
   headT = n;
   return headT;
}

void get_recv_bytes(Tasks t){
    int i;
    if(t->flag == -1)
      return;
    Tasks next = t->next;
    for(i = 0; i < WORKERS; i++)
        printf("%ld ", *(t->recv_bytes)[i]);
    while(next!=NULL){
       for(i = 0; i < WORKERS; i++)
          printf("%ld ", *(next->recv_bytes)[i]);
       next = next->next;
    }
}
