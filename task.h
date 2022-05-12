#include "workerh2.h"

struct task_node{
  pid_t *pid;
  long **recv_bytes;
  int *shmid;
  int flag;
  long *prev_goodput;
  struct task_node *next;
};

typedef struct task_node * Tasks;

struct thread_arg{
  int argc; 
  char **argv;
  int *syncdata;
  pthread_mutex_t *sync_lock;
  int id;
  long *recv_bytes;
  pthread_t *thid;
  struct thread_arg *next;
};

typedef struct thread_arg * TTasks;

Tasks init_tasks_list(void);
Tasks addTask(Tasks headT);
Tasks delTask(Tasks headT);
void get_recv_bytes(Tasks t);
long compute_moving_avg(void);
void *threadworker(void *arg);

TTasks init_Ttasks_list(void);
