#include <stdio.h>
#include <stdlib.h>

void printMe(struct shmData pcb);
void qOne(int index, int *array);
void insertShift(int start, int end, int *array);

typedef struct processControlBlock{
	int queue;	
	int totCPUTime;
  int totSysTime;
  int prevBurstTime;
}PCB;

typedef struct ShmData{             //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory
  int seconds;
  int nanoseconds;
  struct processControlBlock PCB[18];
}shmData;

int main(int argc, char *argv[]){
	int  array[8]= {0, 0, 0, 1, 1, 1, 2, 2};


	  for(int i = 0;i<18;i++){
	  	shmData.PCB[i].totCPUTime = i;
	  }
	
	printMe(shmData);
	// int z = 1;
	// int q = 2;
	// qOne(z, array);
	// printf("-----------------------------------------------------------\n");
	// print(array);

	// printf("-----------------------------------------------------------\n");

	// qOne(q, array);
	// print(array);

	//move array[0] to the end of q1

}



void printMe(struct ShmData pcb){
	// for(int i=0;i<5;i++){
	// 	printf("%d\n", a[i]);
	// }
	int flag =1;
	int i = 0;
	// while((i < 8) && flag){
	// 	if(a[i] == 2){
	// 		flag = 0;
	// 		//printf("Found 2. |||| %d\n", a[i]);
	// 		printf("The end of q1 is arr[%d]\n", i-1);
	// 	}else{
	// 		printf("Array[%d] = %d\n", i, a[i]);
	// 		i++;
	// 	}	
	// }
	for(int x =0; x<18;x++){
		printf("array[%d] = %d\n", x, pcb[x].PCB.totCPUTime);
	}
}

void qOne(int index, int *array){
	//move from q0 to end of q1
	//index is the pos of the element we want to move
	int flag =1;
	int i = 0;				//where we need to move it to
	while((i < 8) && flag){
		if(array[i] == 2){
			insertShift(index, i-1, array);
			flag =0;
		}else{
			i++;
		}	
	}

}

void insertShift(int start,int end, int *array){
	//index to move to
	int intHolder = array[start];
	for(int x = start;x<=end;x++){
		array[x] = array[x+1];
	}
	intHolder = 1;
	array[end] = intHolder;
}