#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

#define alpha 2
#define bravo 3
#define waitThreshhold 500000000
#define qua 4
#define SEM_NAME "/sem"

void exitfuncCtrlC(int sig);


typedef struct processControlBlock{
  int queue;  
  int totCPUTime;
  int totSysTime;
  int prevBurstTime;
  int waitTime;
  pid_t pid;
  int ready;                  //lets the process know when to run
  int ranSchedNum;            //between [0,3] to determine certain things
  int quantum;
}PCB;

typedef struct ShmData{             //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory
  int seconds;
  int nanoseconds;
  int pSeconds;
  int pNanoseconds;
  struct processControlBlock PCB[18];
  int bitVector[18];
  int currentProcessCount;
  int ranSchedNum;
}shmData;

shmData shm;
shmData *shmPtr;

int main(int argc, char * argv[]){

	int shmKey = 3690;
	int id;

  if (signal(SIGINT, exitfuncCtrlC) == SIG_ERR) {
       printf("SIGINT error\n");
       exit(1);
   }

	if ((id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666)) < 0){
       perror("SHMGET:User");
       exit(1);
   }

   // id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666);
   //    if (id < 0){
   //      perror("SHMGET:User");
   //      exit(1);
   //    }

   if((shmPtr = shmat(id, NULL, 0)) == (shmData *) -1){
       perror("SHMAT");
       exit(1);
   }


    sem_t *semaphore = sem_open(SEM_NAME, O_RDWR);
    if (semaphore == SEM_FAILED) {
        perror("sem_open(3) failed in child");
        exit(EXIT_FAILURE);
    }



   // for(int i = 0;i<18;i++){
   // 	printf("PCB#: %d \t cpuTime: %d\n", shmPtr->PCB[i].queue, shmPtr->PCB[i].totCPUTime);
   // }
   printf("inside child proces:  PID: %d\n", getpid());
   shmdt(shmPtr);
   return 0;

}

void exitfuncCtrlC(int sig){

    fprintf( stderr, "Child %ld is dying from parent\n", (long)getpid());
    shmdt(shmPtr);
    //sem_unlink(SEM_NAME);
    exit(1);
}