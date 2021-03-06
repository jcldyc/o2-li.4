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

#define billion 1000000000
#define maxTotalProc 100
#define alpha 2
#define bravo 2
#define waitThreshhold 500000000
#define qua 4

// semaphore globals
#define SEM_NAME "/semo"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1
#define CHILD_PROGRAM "./user"

//function declarations

void ChildProcess(void);
void ctrlPlusC(int sig);
int rand02(void);
int pcbEmpty(int *bitVector);

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
}shmData;

shmData shm;
shmData *shmPtr;

void qOne(int index, struct processControlBlock *array);
void qTwo(int index, struct processControlBlock *array);
void insertShift(int start, int end, struct processControlBlock *array, int queueNum);
void qZero(int index, struct processControlBlock *PCB);

int main(int argc, char *argv[]){
  int option;
  char logFile[] = "logFile";         //set the name of the default log file
  int custLF = 0;
  int runIt = 1;                      //flag to determine if program should run.
  FILE* file_ptr;
  pid_t pid;
  int shmKey = 3690;			                //this  is the key used to identify the shared memory

  shmPtr = &shm;			                 //points the shared memory pointer to teh address in shared memory
  int id;
  clock_t start_t = clock();
  


  //----------------------getOpt-------------------------------------------------------------------
    while((option=getopt(argc, argv, "hl:")) != -1){
      switch(option){
        case 'h':               //this option shows all the arguments available
          runIt = 0;            //how to only asked.  Set to 0 *report to log file*
          printf(" \t./oss: \n\t\t[-l (Log File Name)]\n\t\t[-t (Time to Terminate)] \n\t\t [-i (Amount of nanoseconds added each loop)]\n");
          break;

        case 'l':               //option to name the logfile used
          custLF = 1;

          if(custLF){
            file_ptr = fopen(optarg, "w");
            fclose(file_ptr);
            file_ptr = fopen(optarg, "a");
            printf("\tLog File used: %s\n", optarg);
          }
          break;
        default:
          printf("\tno option used \n");
          break;
      }
    }

    //create the log file specified by user
  //otherwise, use the default log file name : logFile

  if(!custLF && runIt){
    file_ptr = fopen(logFile, "w");
    fclose(file_ptr);
    file_ptr = fopen(logFile, "a");
    printf("\tLog File name: %s\n", logFile);
  }
  printf("-----------------------------------------------------------------------\n\n");

  if (signal(SIGINT, ctrlPlusC) == SIG_ERR) {
        printf("SIGINT error\n");
        exit(1);
    }

  if(runIt){

    printf("size of shm: %d\n", (int)sizeof(shm));

    //only creates the shared memory if the program runs through for(runIt)
      id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666);
      if (id < 0){
        perror("SHMGET:oss");
        exit(1);
      }

      shmPtr = shmat(id, NULL, 0);
      if(shmPtr == (shmData *) -1){
        perror("SHMAT");
        exit(1);
      }


      /* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
       sem_t *semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);

       if (semaphore == SEM_FAILED) {
           perror("sem_open(3) error");
           exit(EXIT_FAILURE);
       }




          /* Close the semaphore as we won't be using it in the parent process */
       if (sem_close(semaphore) < 0) {
           perror("sem_close(3) failed");
           /* We ignore possible sem_unlink(3) errors here */
           sem_unlink(SEM_NAME);
           exit(EXIT_FAILURE);
       }

      






       int needMoreProcesses = 1;
       int rand02 = rand() % (2 + 1 -0) + 0;  //int between 0 & 2;
       //rand02 = rand02();
       int newProcTime[2] ={rand02, shmPtr->nanoseconds};   //[0] holds seconds [1] holds nano
       int currentProcessCount = 0;
       //int bitVector[18];
       for(int z=0;z<18;z++){
        shmPtr->bitVector[z] = 1;
       }


       while(needMoreProcesses){

         int shmSecond = shmPtr->seconds;           //gets seconds & nanoseconds in shm
         int shmNano = shmPtr->nanoseconds;
         printf("TIME: Sec: %d Nan: %d\n", shmSecond, shmNano);
         printf("P-TIME: Sec: %d Nan: %d\n", shmPtr->pSeconds, shmPtr->pNanoseconds);


         //create a new process if
         //shmSecond = current seconds so if it's greater than the stored newProcTime seconds, new  process
         //or if seconds are the same and shmNano is bigger than newProcTimeNano, newprocess

         if(newProcTime[0]<shmSecond || (newProcTime[0]==shmSecond && newProcTime[1] < shmNano)){
        

          int bvFull = 1;
          int emptyFlag = 1;
          int index;
          int i = 0;
          
          while(emptyFlag && i<18){
            if(shmPtr->bitVector[i] == 1){
              bvFull = 0;
              index = i;
              emptyFlag=0;
            }i++;
          }

          if(!bvFull){
            if((shmPtr->PCB[index].pid = fork()) == 0){
              printf("Child created at PCB[%d]\n", index);
              shmPtr->bitVector[index]=0;
              shmPtr->currentProcessCount++;
              shmPtr->PCB[index].ranSchedNum = rand() % (3 + 1 - 0) + 0;
              shmPtr->PCB[index].quantum = qua;
              ChildProcess();
            }else if(shmPtr->PCB[index].pid < 0){
              printf("Child failed to fork. \n");
            }
          }

          // for(int z = 0;z<18;z++){
          //   printf("bv[%d] = %d || queue = %d\n", z, shmPtr->bitVector[z], shmPtr->PCB[z].queue);
          // }
          // printf("currentProcessCount: %d", shmPtr->currentProcessCount);
         }

         //queue and scheduling section
         //First I'm going to cycle through bitVector to see which PCB has been initiliazed
         //then run through only those initialized and set it's queue if it needs to be changed.
         //Upon scheduling, I need to create a random number between [0,3]

         //getting the avg wait time in  queue 1 and 2
         int sumHolder1, sumHolder2, amtOfQ1, amtOfQ2;
         float qOneAvg, qTwoAvg;

         for(int x = 0; x<18;x++){
            if(!shmPtr->bitVector[x] && shmPtr->PCB[x].queue == 1){
                sumHolder1 += shmPtr->PCB[x].waitTime;
                amtOfQ1++;
            }
            if(!shmPtr->bitVector[x] && shmPtr->PCB[x].queue == 2){
                sumHolder2 += shmPtr->PCB[x].waitTime;
                amtOfQ2++;
            }
         }
         qOneAvg = (float)sumHolder1/(float)amtOfQ1;
         qTwoAvg = (float)sumHolder2/(float)amtOfQ2;

         //moves process from q0 to q1 if it's wait time is higher than threshhold and q1 avg wait time * alpha
         //                   q1 to q2                                                 q2 avg wait time * bravo

         for(int q=0;q<18;q++){
          if(!shmPtr->bitVector[q] && shmPtr->PCB[q].queue == 0){               //It would be better to change it to 0 for empty and 1 for initialized
            if(shmPtr->PCB[q].waitTime > waitThreshhold && (float)shmPtr->PCB[q].waitTime > (qOneAvg*alpha)){
                shmPtr->PCB[q].queue = 1;
                shmPtr->PCB[q].ranSchedNum = rand() % (3 + 1 - 0) + 0;
                shmPtr->PCB[q].quantum = qua/2;
            }
          }
          if(!shmPtr->bitVector[q] && shmPtr->PCB[q].queue == 1){               //It would be better to change it to 0 for empty and 1 for initialized
            if(shmPtr->PCB[q].waitTime > waitThreshhold && (float)shmPtr->PCB[q].waitTime > (qTwoAvg*bravo)){
                shmPtr->PCB[q].queue = 2;
                shmPtr->PCB[q].ranSchedNum = rand() % (3 + 1 - 0) + 0;
                shmPtr->PCB[q].quantum = qua/4;
            }
          }
         }















         //-------------LOGICAL CLOCK ++ ------------------------------------------
         //add time to clock at end of loop by adding rand(0-1000) nanoseconds to clock
         int addToNano = rand() % (1000 + 1 -0) + 0;    //what's added to nanoseconds
         shmPtr->seconds++;
         if((addToNano % billion) != addToNano){        //determines whether to increase seconds or nano
          //  shmSeconds++;
          //  shmNano = newNanoTime % billion;

           shmPtr->seconds++;
           shmPtr->nanoseconds = addToNano % billion;
         }else{
           shmPtr->nanoseconds += addToNano;
         }


         //-------------PROCESS CLOCK ++---------------------------------

         addToNano = rand() %(3000000 + 1 -0) + 2000000;
         if((addToNano % billion) != addToNano){        //determines whether to increase seconds or nano
          //  shmSeconds++;
          //  shmNano = newNanoTime % billion;

           shmPtr->pSeconds++;
           shmPtr->pNanoseconds = addToNano % billion;
         }else{
           shmPtr->pNanoseconds += addToNano;
         }


         //quits creating processes when 100 is created
         if(shmPtr->currentProcessCount >= 100){
         	needMoreProcesses = 0;
         }
         sleep(1);
        
       }
       sleep(1);













  }//runIt end







  //----------------------------------------------------------------------------------------------------
  if (sem_unlink(SEM_NAME) < 0)
      perror("sem_unlink(3) failed");

  shmdt(shmPtr);

  return 0;

}

void ChildProcess(void){
    char *args[]={"./user",NULL};
    execvp(args[0],args);
}

void ctrlPlusC(int sig){

    fprintf( stderr, "Child %ld is dying from parent\n", (long)getpid());
    shmdt(shmPtr);
    sem_unlink(SEM_NAME);
    exit(1);
}

int rand02(void){
  int x = rand() % (2 + 1 -0) + 0;  //int between 0 & 2
  return x;
}


