struct saturation {
 	long rate;
	int flows ;  
	int error;
	int oksaturation;

};

typedef struct saturation SaturateResult;

SaturateResult saturate(char *largeurl,char *uploadurl);
//int start_workers(pid_t *pid, int *shmid, long **recv_bytes, char *argv[]);*/



