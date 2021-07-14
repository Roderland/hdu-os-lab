#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>

int main()
{
	while(1) {
		char command[1024];
		pid_t pid;
		printf("shell: ");
		gets(command);
	
		if (strcmp(command, "exit")==0) {
			exit(0);
		} else if (strcmp(command, "cmd1")==0 || strncmp(command, "cmd2", 4)==0 || strcmp(command, "cmd3")==0) {
			pid=vfork();
			if (pid<0) {
				printf("ERROR\n");
				exit(-1);
			} else if (pid==0) {
				execl(command, "", NULL);
				break;
			} else {
				pid_t temp = wait(NULL);
			}
		} else {
			system(command);
		}
	}
	return 0;
}

















