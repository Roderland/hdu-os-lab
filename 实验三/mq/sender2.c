#include "ipc.h"

int main()
{
    int msgid;//消息队列标识符

    printf("输入消息队列标识符: ");
    scanf("%d", &msgid);
    

    init_signal();

    char str[100];
    struct my_msgbuf s_msg;//消息发送区
    s_msg.mtype = 1;
    s_msg.sendid= 2;

    //不断发送
    while(1)
    {
        memset(str, '\0', strlen(str));

	printf("sender2发送: ");
        scanf("%s", str);
        
        sem_wait(sem_send);

        if(strcmp(str, "exit") == 0)
        {
            strcpy(s_msg.mtext, "end2");
            msgsnd(msgid, &s_msg, sizeof(struct my_msgbuf), 0);
            sem_post(sem_receive);
            break;
        }

        strcpy(s_msg.mtext, str);
        //printf("%s", s_msg.mtext);
        msgsnd(msgid, &s_msg, sizeof(struct my_msgbuf), 0);

        sem_post(sem_receive);
    }

    sem_wait(sem_over2);
    struct my_msgbuf r_msg;
    msgrcv(msgid, &r_msg, sizeof(struct my_msgbuf), 0, 0);
    printf("sender2退出: %s\n", r_msg.mtext);

    sem_post(sem_send);
    return 0;
}
