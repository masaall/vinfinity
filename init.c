
#include "types.h"
#include "user.h"
#include "fcntl.h"

int main(void){

	int pid, i;

	if (open("console", O_RDWR) < 0){
		mknod("console", 1, 1);
		open("console", O_RDWR);
	}

	dup(0);
	dup(0);

	printf("init starting \n");

	for (i = 0; i < 2; i++){
		pid = fork();
		if (pid > 0){
			printf("parent %d creating child %d\n", getpid(), pid);
			pid = wait();
			printf("child %d done \n", pid);
		} else if (pid == 0){
			printf("child %d exiting\n", getpid());
			exit();
		} else {
			printf("fork error\n");
		}
	}
	printf("%p \n", malloc(16));
	printf("%p \n", malloc(16));
	printf("%p \n", malloc(16));
	printf("%p \n", malloc(16));
	printf("%p \n", malloc(16));
	
	for (;;);
}
