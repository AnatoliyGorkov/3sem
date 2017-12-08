#include "common.h"

int main(int argc, char** argv)
{
    key_t key = ftok("keyfile.txt", CHAT_KEY);
    int msgid = msgget(key, IPC_CREAT | 0666);
    
    char fifoName[FIFONAMESIZE];
    int i = 1;
    sprintf(fifoName, FIFONAMETEMPLATE, i);
    // printf("1\n");
    while(mknod(fifoName, S_IFIFO | 0666, 0) == -1)
    {
        if (errno != EEXIST)
            return 3;
        i++;
        sprintf(fifoName, FIFONAMETEMPLATE, i);
    }
    // printf("1\n");

    ChatConnect chatConnect = {CHAT_CHANNEL, i};
    msgsnd(msgid, &chatConnect, sizeof(chatConnect.n), 0);
    // printf("1\n");
    int fifofd = open(fifoName, O_RDONLY);
    Buffer* buffer = bufferConstruct(BUFFERSIZE);
    while(1)
    {
        getMessage(fifofd, buffer);
        printf("<%s>: %s", buffer -> buffer, buffer -> buffer + MAXLOGINSIZE);

    }

    return 0;
}