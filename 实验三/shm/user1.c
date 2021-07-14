#include "shmdata.h"

int main()
{
	exit_signal();
	init_signal();
	int running = 1;   
    void *shm = NULL;  
    struct shared_use_st *shared = NULL;
   	char w_buffer[BUFSIZ + 1];//用于保存输入的文本
   	char r_buffer[BUFSIZ + 1];
    int shmid;  //创建共享内存
    shmid = shmget((key_t)1234, sizeof(struct shared_use_st), 0666|IPC_CREAT);
    if(shmid == -1)
    {  
        fprintf(stderr, "shmget failed\n");
        exit(EXIT_FAILURE);
    }   //将共享内存连接到当前进程的地址空间
    shm = shmat(shmid, (void*)0, 0);
    if(shm == (void*)-1)
    {  
        fprintf(stderr, "shmat failed\n");     
        exit(EXIT_FAILURE);
    }  
    shared = (struct shared_use_st*)shm; 
    
      
    while (running) 
    {
    	if(w_buffer[0]!='\0')
    	{
    		if (sem_trywait(sem_write1)==0)
			{
				strncpy(shared->text, w_buffer, TEXT_SZ); 
        		sem_post(sem_read2);
        		if (strncmp(w_buffer, "over", 4) == 0) 
    			    break;
    			memset(w_buffer, '\0', sizeof(char));
    		}
    	}
    	printf("You send: ");
    	if (kbhit()) 
    	{
			fgets(w_buffer, BUFSIZ, stdin);
			if (sem_trywait(sem_write1)==0)
			{
				strncpy(shared->text, w_buffer, TEXT_SZ); 
        		sem_post(sem_read2);
        		if (strncmp(w_buffer, "over", 4) == 0) 
    			    break;
    			memset(w_buffer, '\0', sizeof(char));
    		}
        }
        else 
        {
        	printf("\r");
        	while (sem_trywait(sem_read1)==0)
        	{
        		strncpy(r_buffer, shared->text, TEXT_SZ);
    			printf("You receive: %s", r_buffer);
    			sem_post(sem_write1);
    	        if (strncmp(r_buffer, "over", 4) == 0) 
    		    	running = 0;
    		}
    	}
    }
    
    
    if(shmdt(shm) == -1)   
    {      
        fprintf(stderr, "shmdt failed\n");     
        exit(EXIT_FAILURE);
    }  
    if(!running) //谁后退出谁删除共享内存
    {
    	if(shmctl(shmid, IPC_RMID, 0) == -1)   
    	{  
       		fprintf(stderr, "shmctl(IPC_RMID) failed\n");  
        	exit(EXIT_FAILURE);
    	}  
    	printf("You close the shm.\n");
    }
    sleep(1);
    exit_signal();  
    exit(EXIT_SUCCESS);
}
    
    	
