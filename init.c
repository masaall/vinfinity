
#include "types.h"
#include "user.h"
#include "memlayout.h"

void uint8(void){

	printf("unsigned char\n");
	unsigned char n = -1;

	printf("%d %ld\n", n, n);
	printf("%x %lx\n", n, n);
}

void int8(void){
	printf("signed char\n");
	char n = -1;

	printf("%d %ld\n", n, n);
	printf("%x %lx\n", n, n);
	
}
/*
int main(void){

//	uint8();
//	int8();

	for (;;);
}
*/

int main(void){

//	for (;;);

	int pid;

	for (int i = 0; i < 5; i++){
		pid = fork();
		if (pid > 0){
			printf("parent %d creating child %d \n", getpid(), pid);
			pid = wait();
			printf("child %d exiting\n", pid);
		} else if (pid == 0){
			printf("child %d created \n", getpid());
			exit();
		} else {
			printf("fork error\n");
		}		
	}
	for (;;);
}

