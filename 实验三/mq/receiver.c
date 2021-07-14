#include "ipc.h"

int main()
{
    int msgid;//消息队列标识符
    msgid = msgget(IPC_PRIVATE, 0666|IPC_CREAT);//创建获取消息队列

    printf("消息队列标识符: %d\n", msgid);

    sem_unlink("send");
    sem_unlink("receive");
    sem_unlink("over1");
    sem_unlink("over2");

    init_signal();

    struct my_msgbuf r_msg;//消息接受区
    struct my_msgbuf s_msg;
    s_msg.mtype = 2;
    s_msg.sendid = 3;
    int flag_over1 = 0;
    int flag_over2 = 0;

    while (1)
    {
        sem_wait(sem_receive);

        msgrcv(msgid, &r_msg, sizeof(struct my_msgbuf), 0, 0);
        printf("receiver收到sender%d的消息: %s\n", r_msg.sendid, r_msg.mtext);


        if (r_msg.sendid == 1)
        {
            if (strcmp(r_msg.mtext, "end1") == 0)
            {
                //printf("发送over1\n");
                strcpy(s_msg.mtext, "over1");
                msgsnd(msgid, &s_msg, sizeof(struct my_msgbuf), 0);

                sem_post(sem_over1);
                flag_over1 = 1;
            }
            else
            {
                sem_post(sem_send);
            }
        }
        else if(r_msg.sendid == 2)
        {
            if (strcmp(r_msg.mtext, "end2") == 0)
            {
                //printf("发送over2\n");
                strcpy(s_msg.mtext, "over2");
                msgsnd(msgid, &s_msg, sizeof(struct my_msgbuf), 0);

                sem_post(sem_over2);
                flag_over2 = 1;
            }
            else
            {
                sem_post(sem_send);
            }
        }

        if (flag_over1 && flag_over2)
            break;
    }


    sem_unlink("send");
    sem_unlink("receive");
    sem_unlink("over1");
    sem_unlink("over2");

    return 0;
}
