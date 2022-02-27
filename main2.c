#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "task.h"

pthread_mutex_t sync_lock;

int main(int argc, char *argv[]) {
  pthread_t thid[WORKERS];
  void *ret;
  int i = 0;
  int *syncdata = (int*)malloc(sizeof(int));
  *syncdata = 0;
  TTasks t;
  for(i = 0; i < WORKERS; i++ ){
     char *tworker_argv[] = {argv[0], argv[1], argv[2], NULL};
     struct thread_arg *targv = (struct thread_arg *)malloc(sizeof(struct thread_arg));
     targv->argc = argc;
     targv->argv = tworker_argv;
     targv->syncdata = syncdata;
     targv->sync_lock = &sync_lock;
     targv->id = i;
     targv->recv_bytes = (long*)malloc(sizeof(long));
     *targv->recv_bytes = 0;
     if (pthread_create(&thid[i], NULL, threadworker, targv) != 0) {
        perror("pthread_create() error");
        exit(1);
     }
     printf("%d \n", i);
  }
  
  for(i = 0; i < WORKERS; i++){ 
     if (pthread_join(thid[i], &ret) != 0) {
        perror("pthread_create() error");
        exit(3);
     }
  }

  printf("thread exited with '%s'\n", (char*)ret);
  return 0;
}
