#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/wait.h>

int main(){
	
	char *name = "foo";
     
    int rc = fork();
	if (rc == 0){// child
  		//close(1);
  		// int fd = open("output.txt", O_RDWR|O_CREAT);
		// execute a program
		char *myargs[] = {"ls", "/Users/pjavinash/Documents/Avinash/UTD_MS/3rd_Sem/CS5348/Projects/P1/CS5348UnixShellP1/testdata", 0};
		execv("/bin/ls", myargs);
		printf("EXEC FAILED");
	}
	else { // parent takes this path
	//	printf("PARENT: I am waiting for my child to finish \n");
		int ret = wait(NULL);
	//	printf("PARENT: Child %d finished \n", ret);
	}
    return 0;
}