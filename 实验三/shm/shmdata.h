#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/ipc.h>
#include <termios.h>

#ifndef _SHMDATA_H_HEADER
#define _SHMDATA_H_HEADER
#define TEXT_SZ 2048

sem_t *sem_read1;
sem_t *sem_read2;
sem_t *sem_write1;
sem_t *sem_write2;

struct shared_use_st
{  
    char text[TEXT_SZ];//记录写入和读取的文本
};

void init_signal()
{
    //初始化信号量
    sem_read1 = sem_open("read1", O_CREAT, 0666, 0);
    sem_read2 = sem_open("read2", O_CREAT, 0666, 0);
    sem_write1 = sem_open("write1", O_CREAT, 0666, 1);
    sem_write2 = sem_open("write2", O_CREAT, 0666, 1);
}

void exit_signal()
{
	sem_unlink("read1");
    sem_unlink("read2");
    sem_unlink("write1");
    sem_unlink("write2");    
}

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if(ch != EOF && ch != '\n')
    {
    	printf("%c", ch);
    	ungetc(ch, stdin);
        return 1;
    }
    return 0;
}
#endif
