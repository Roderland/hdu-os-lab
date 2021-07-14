#define _GNU_SOURCE
#include <unistd.h>
#include<sys/syscall.h>
#include<stdio.h>
#include<stdlib.h>
int main(){
    pid_t pid;
    int nicevalue;
    int flag;
    int p = 0;
    int n = 0;
    int *prio;
    int *nice;
    prio = &p;
    nice = &n;

    printf("请输入pid：");
    scanf("%d",&pid);

    printf("请输入nice：");
    scanf("%d",&nicevalue);

    syscall(338,pid,nicevalue,prio,nice);

    printf("现在的nice为%d\n",n);
    printf("pri : %d\n", p);

    return 0;
}
