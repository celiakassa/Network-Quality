#define MEMSIZE 4096 /* taille de la mémoire partagée*/

int init(pid_t pid, int proj_id );
int mem_attach(int shmid,void **addr);
void mem_detach(void **adresse);
