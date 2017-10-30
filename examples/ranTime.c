#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]){
	int random;
	for(int i=0; i<20;i++){
		random = rand() % (2 + 1 - 0) + 0;
		printf("random number between 0 and 2: %d\n", random);
	}
	return 0;

}
