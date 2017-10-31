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

// semaphore globals
#define SEM_NAME "/sem"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1
#define CHILD_PROGRAM "./user"

//function declarations

void ChildProcess(void);
void ctrlPlusC(int sig);
int rand02(void);
int pcbEmpty(void);

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
  int option;
  char logFile[] = "logFile";         //set the name of the default log file
  int custLF = 0;
  int runIt = 1;                      //flag to determine if program should run.
  FILE* file_ptr;
  pid_t pid;
  int shmKey = 3699;			                //this  is the key used to identify the shared memory

  shmPtr = &shm;			                 //points the shared memory pointer to teh address in shared memory
  int id;
  clock_t start_t = clock();
  int bitVector[18];


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

       for(int x = 0; x<18; x++){
         shmPtr->PCB[x].totCPUTime=x;
       }

       int mainLoop = 1;
       int rand02 = rand02();
       int newProcTime[2] ={rand02, shmPtr->nanoseconds};   //[0] holds seconds [1] holds nano


       while(mainLoop){
         int shmSecond = shmPtr->seconds;           //gets seconds & nanoseconds in shm
         int shmNano = shmPtr->nanoseconds;

         //create a new process if the new seconds is greater than clock

         if((newProcTime[0]<shmSecond) || (newProcTime[0]==shmSecond && newProcTime[1] < shmNano)){
           int flag = 1;
           int iter = 0;
           //make sure there's an open process block

           while(flag){
             if(bitVector[iter] == 0){
               //create new process.  Passed time checkmark and there's an open PCB
               //we'll have the pcb array number so we can initialize
               //we need to fork off a process and execl
               //****fill shmPtr->PCB[iter].pid = fork();
               flag = 0;
             }else if(iter ==17){
               flag = 0;
               printf("All PCB's have been used");
             }else iter++;
           }
         }








         //add time to clock at end of loop by adding rand(0-1000) nanoseconds to clock
         int addToNano = rand() % (1000 + 1 -0) + 0;    //what's added to nanoseconds
         if((addToNano % billion) != addToNano){        //determines whether to increase seconds or nano
          //  shmSeconds++;
          //  shmNano = newNanoTime % billion;
           shmPtr->seconds++;
           shmPtr->nanoseconds = addToNano % billion;
         }else{
           shmPtr->
           nanoseconds += addToNano;
         }

         if(shmPtr->seconds > 1 && pcbEmpty()){
           mainLoop = 0;
         }
       }













  }//runIt end

  //----------------------------------------------------------------------------------------------------
  if (sem_unlink(SEM_NAME) < 0)
      perror("sem_unlink(3) failed");

  return 0;

}

void ChildProcess(void){
    char *args[]={"./user",NULL};
    execvp(args[0],args);

    // if (execl(CHILD_PROGRAM, CHILD_PROGRAM, NULL) < 0) {
    //   perror("execl(2) failed");
    //   exit(EXIT_FAILURE);
    // }
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

//returns 1 if pcb/bitVector is empty; else return 0 because it isn't empty
int pcbEmpty(void){
  for(int x = 0;x<18;x++){
    if(bitVector[x]==1){
      return 0;
    }
  }
  return 1;
}
