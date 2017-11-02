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




typedef struct processControlBlock{
  int totCPUTime;
  int totSysTime;
  int prevBurstTime;
  int queue;
}PCB;


typedef struct ShmData{             //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory
  int seconds;
  int nanoseconds;
  struct processControlBlock PCB[18];
}shmData;

shmData shm;
shmData *shmPtr;

void ChildProcess(void);
void qOne(int index, struct processControlBlock *array);
void qTwo(int index, struct processControlBlock *array);
void insertShift(int start, int end, struct processControlBlock *array, int queueNum);


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
	  	shmPtr->PCB[i].queue=0;
	  }
	   qTwo(7,shmPtr->PCB);

	  for(int s=0;s<18;s++){
	  	printf("PCB[%d].queue= %d\n", s, shmPtr->PCB[s].queue);
	  }
	  printf("=-------------------------------------------------------\n");

	  qOne(5, shmPtr->PCB);
	  for(int s=0;s<18;s++){
	  	printf("PCB[%d].queue= %d\n", s, shmPtr->PCB[s].queue);
	  }


	  
	 


	  printf("=-------------------------------------------------------\n");

	   qTwo(12,shmPtr->PCB);

	  for(int s=0;s<18;s++){
	  	printf("PCB[%d].queue= %d\n", s, shmPtr->PCB[s].queue);
	  }


	  printf("=-------------------------------------------------------\n");

	  qOne(3, shmPtr->PCB);
	  for(int s=0;s<18;s++){
	  	printf("PCB[%d].queue= %d\n", s, shmPtr->PCB[s].queue);
	  }


	  printf("=-------------------------------------------------------\n");
	  // qOne(10, shmPtr->PCB);
	  // qOne(15, shmPtr->PCB);
	  // // qOne(16, shmPtr->PCB);

	  // for(int s=0;s<18;s++){
	  // 	printf("PCB[%d].queue= %d\n", s, shmPtr->PCB[s].queue);
	  // }


	  // printf("=-------------------------------------------------------\n");

      //---------------------------------------------------

	pid_t pid;

	if((pid=fork())==0){
		pid = getpid();
		printf("Welcome to child process.  PID : %d\n", pid);
		ChildProcess();
	}else{
		pid=getpid();
		printf("Parent process. PID %d\n", pid);

	}
	sleep(1);
	shmdt(shmPtr);
	return 0;

}


void ChildProcess(void){
    char *args[]={"./child",NULL};
    execvp(args[0],args);
}

void qOne(int index, struct processControlBlock *PCB){
	//move from q0 to end of q1
	//index is the pos of the element we want to move
	int flag =1;
	int i = 0;				//where we need to move it to
	while(i < 18 && flag){
		//printf("made it here");
		if(PCB[i].queue == 2){							//find where queue 1 ends.  This is where the process willbe placed
			insertShift(index, i-1, PCB, 1);
			flag =0;
		}else if(i==17 && flag){
			insertShift(index, 17, PCB, 1);
			flag = 0;								//doesn't really matter.  Loop will cancel because i=18. 
		}else{
			i++;
		}

	}//insertShift(index, 18, PCB);

}



void qTwo(int index, struct processControlBlock *PCB){ 
	insertShift(index, 17, PCB, 2);
}

void insertShift(int start,int end, struct processControlBlock *PCB, int queueNum){
	//index to move to
	struct processControlBlock pcbHolder = PCB[start];
	for(int x = start;x<=end;x++){
		PCB[x] = PCB[x+1];
	}
	pcbHolder.queue=queueNum;
	PCB[end] = pcbHolder;
}
// //returns 1 if pcb/bitVector is empty; else return 0 because it isn't empty
// int pcbEmpty(int *bitVector){
//   for(int x = 0;x<18;x++){
//     if(bitVector[x]==1){
//       return 0;
//     }
//   }
//   return 1;
// }

// //creates a new process and puts it in queue zero 
// void qZero(int index, struct processControlBlock *PCB){
//   //need to check if PCB is empty
//   if(PCB[0].available){
//     insertShift(index, 0, PCB, 0);
//   }else{

//   	int flag = 1;
//   	int i = 0;
//   	while(i<18 && flag){
//   		if(PCB[i].queue == 1 || PCB[i].queue == 2){  //it'll be checking if there are any in quue 1 && then queue 2
//   			insertShift(index, i-1, PCB, 0);
//   			flag = 0;
//   		}else if(i==17 && flag){
//   			insertShift(index, 17, PCB, 0);
//   			flag = 0;
//   		}else{
//   			i++;
//   		}
//   	}
//   }
// }

// void qOne(int index, struct processControlBlock *PCB){
// 	//move from q0 to end of q1
// 	//index is the pos of the element we want to move
//   printf("inside qOne");
// 	int flag =1;
// 	int i = 0;				//where we need to move it to
// 	while(i < 18 && flag){
// 		//printf("made it here");
// 		if(PCB[i].queue == 2){							//find where queue 1 ends.  This is where the process willbe placed
//       printf("inserting into PCB[%d]", i);
// 			insertShift(index, i-1, PCB, 1);
// 			flag =0;
// 		}else if(i==17 && flag){
//       printf("inserting into last spot\n");
// 			insertShift(index, 17, PCB, 1);
// 			flag = 0;								//doesn't really matter.  Loop will cancel because i=18. 
// 		}else{
// 			i++;
// 		}

// 	}//insertShift(index, 18, PCB);

// }



// void qTwo(int index, struct processControlBlock *PCB){ 
// 	insertShift(index, 17, PCB, 2);
// }

// void insertShift(int start,int end, struct processControlBlock *PCB, int queueNum){
// 	//index to move to
// 	struct processControlBlock pcbHolder = PCB[start];
// 	for(int x = start;x<=end;x++){
// 		PCB[x] = PCB[x+1];
// 	}
// 	//pcbHolder.queue=queueNum;
// 	PCB[end] = pcbHolder;
//   PCB[end].queue = 2;
// }