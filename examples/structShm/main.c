#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <errno.h>
#include <time.h>

void ChildProcess(void);


typedef struct processControlBlock{
  int totCPUTime;
  int totSysTime;
  int prevBurstTime;
}PCB;


typedef struct ShmData{             //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory
  int seconds;
  int nanoseconds;
  struct processControlBlock PCB[18];
}shmData;


shmData shm;
shmData *shmPtr;


int main(int argc, char *argv[]){

	int shmKey = 3699;
	int id;

	// shared memory allocation

	 //only creates the shared memory if the program runs through for(runIt)
	  id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666);
	  if (id < 0){
	    perror("SHMGET");
	    exit(1);
	  }

	  shmPtr = shmat(id, NULL, 0);
	  if(shmPtr == (shmData *) -1){
	    perror("SHMAT");
	    exit(1);
	  }


	  for(int i = 0;i<18;i++){
	  	shmPtr->PCB[i].totCPUTime = i;
	  }


      //---------------------------------------------------

	pid_t pid;

	if((pid=fork())==0){
		printf("Welcome to child process.  PID : %d\n", pid);
		ChildProcess();
	}else{
		pid=getpid();
		printf("Parent process. PID %d\n", pid);

	}
	return 0;

}


void ChildProcess(void){
    char *args[]={"./child",NULL};
    execvp(args[0],args);
}