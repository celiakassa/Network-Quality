#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <assert.h>

#include "shm.h"

int init(pid_t pid, int proj_id ){
  key_t shm_key; /*clé de la mémoire partagée*/
  int shmid; /* id de la mémoire partagée*/
  char *pathname = (char*)malloc(20);
  sprintf(pathname, "/tmp/%d.key", pid);
  FILE *f = fopen(pathname, "w");
  if(f == NULL)
    return -1;
  fclose(f);
  shm_key = ftok(pathname, proj_id); // création de la clé
  if(shm_key < 0){ // la clé n'a pas pu être créée
    fprintf(stderr, "Impossible de créer la clé:: (%s)\n", strerror(errno));
    return shm_key;
  }
  /* Création de la mémoire partagée */
  shmid = shmget(shm_key, MEMSIZE, IPC_EXCL | IPC_CREAT | 0666); 
  if (shmid<0){ 
    if(errno!=EEXIST){
      fprintf(stderr, "Impossible de créer la mémoire partagée (%s), %d\n", strerror(errno), errno);
      return shmid;
    }
    /* Si la mémoire partagée existe déjà , il faut obtenir son id*/
    shmid = shmget(shm_key, MEMSIZE, 0666);
    return shmid;
  }
  return shmid;
}


int mem_attach(int shmid,void **addr){
  *addr=(long *)shmat(shmid, NULL,0);
  if (*addr==(void *) -1) return -1;
  return 0;
}

void mem_detach(void **adresse){
  shmdt(*adresse);
}
