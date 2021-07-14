#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <string.h>

int main(){
	char name[64];
	char res[64];
	printf("please enter you new name: ");
	scanf("%s", name);
	int len = strlen(name);
	syscall(337, name, len, res);
	printf("修改成功, %s\n", res);	
}

