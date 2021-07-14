#include <stdio.h>
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#define FIFO "./fd"

int main() 
{
	
	int res, fd;
	sem_t *write1, *read1, *read2, *read3;
	int len=0;
	
	pid_t pid1;
    pid_t pid2;
    pid_t pid3;   
	
	sem_unlink("write1");
	sem_unlink("read1"); 
	sem_unlink("read2"); 
	sem_unlink("read3");
	        
    write1 = sem_open("write1", O_CREAT, 0666, 1);
    read1 = sem_open("read1", O_CREAT, 0666, 0);
    read2 = sem_open("read2", O_CREAT, 0666, 0);
    read3 = sem_open("read3", O_CREAT, 0666, 0);  
	
	
	
    if (access(FIFO, F_OK)) 
	{
		res = mkfifo(FIFO, 0777);
		if (res==-1) 
		{
        	    perror("mkfifo create error");
        	    exit(1);
        	}
        }
      
      	fd = open(FIFO, O_RDWR);  
             
        pid1 = fork();
    	if(pid1 > 0)
    	{
        	pid2 = fork();
        	if(pid2 > 0)
        	{
        	    pid3 = fork();
        	}
    	}
    	
    	
    	
    	if(pid1 < 0 || pid2 < 0 || pid3 < 0)
    	{
    		sem_unlink("write1");
		    sem_unlink("read1"); 
		    sem_unlink("read2"); 
		    sem_unlink("read3");
    		perror("pid create error");
        	exit(1);
    	}
        
        if(pid1 == 0)
    	{
    		
        	sem_wait(write1);
		fd = open(FIFO, O_RDWR);
		printf("请输入进程1的数据:");
		char buf[1024];
		gets(buf);
        	write(fd, buf, strlen(buf));
        	len+=strlen(buf);
       	printf("pid:%d 进程1写入数据:%s 累积数据长度:%d\n",getpid(),buf,len);
		close(fd);

        	sleep(1);

        	sem_post(write1);
        	sem_post(read1);

        	exit(0);
    	}
    	if(pid2 == 0)
    	{
        	sem_wait(write1);
		fd = open(FIFO, O_RDWR);
        	printf("请输入进程2的数据:");
		char buf[1024];
		gets(buf);
        	write(fd, buf, strlen(buf));
        	len+=strlen(buf);
       	printf("pid:%d 进程2写入数据:%s 累积数据长度:%d\n",getpid(),buf,len);
		close(fd);

        	sleep(1);

        	sem_post(write1);
        	sem_post(read2);


        	exit(0);
    	}
    	if(pid3 == 0)
    	{
        	sem_wait(write1);
        	
        	fd = open(FIFO, O_RDWR);
        	printf("请输入进程3的数据:");
		char buf[1024];
		gets(buf);
		printf("请输入进程3的写入次数:");
		int sum;
		scanf("%d", &sum);
        	int f = 1;
		int t = 1;
		while (f>0 && t<=sum) {
        		f = write(fd, buf, strlen(buf));
        		len+=strlen(buf);
	       	printf("pid:%d 进程3第%d次写入数据:%s 累积数据长度:%d\n",getpid(),t++,buf,len);
		}
		close(fd);

        	sleep(1);

        	sem_post(write1);
        	sem_post(read3);
	
	
        	exit(0);
    	}
    	
    	
   	sem_wait(read1);
        sem_wait(read2);
        sem_wait(read3);        	
        
        char str[1024*8];

        read(fd, str, sizeof(str));
        printf("pid:%d 父进程接收数据:%s\n", getpid(), str);
	close(fd);

        sleep(1);	

        sem_post(read1);
	sem_post(read2);
	sem_post(read3);
	
	
   
    return 0;
}





		
