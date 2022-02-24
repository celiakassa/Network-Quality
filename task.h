#include "worker.h"

struct task_node{
  pid_t *pid;
  long **recv_bytes;
  int *shmid;
  int flag;
  struct task_node *next;
};

typedef struct task_node * Tasks;

Tasks init_tasks_list(void);
Tasks addTask(pid_t *pid, int *shmid, long **recv_bytes, Tasks headT);
void get_recv_bytes(Tasks t);
