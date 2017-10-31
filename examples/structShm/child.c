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

int main(int argc, char * argv[]){

	int shmKey = 3699;
	int id;

	if ((id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666)) < 0){
       perror("SHMGET");
       exit(1);
   }

   if((shmPtr = shmat(id, NULL, 0)) == (shmData *) -1){
       perror("SHMAT");
       exit(1);
   }


   for(int i = 0;i<18;i++){
   	printf("PCB#: %d\n", shmPtr->PCB[i].totCPUTime);
   }

   return 0;

}