#include "common.h"
#include <stdlib.h>

int getInput(Buffer* buffer)
{
    size_t i = 0;
    char c = '0';
    while(1)
    {
        c = getchar();
        if (i >= buffer -> maxSize)
            bufferExpand(buffer);
        *(buffer -> buffer + i) = c;
        if (c == '\n')
            break;
        i++;
    }
    i++;
    *(buffer -> buffer + i) = '\0';

    if (strcmp(buffer -> buffer, STR_EXIT) == 0)
        return TYP_EXIT;
    return TYP_MSG;
}

int main(int argc, char** argv)
{
    Buffer* buffer = bufferConstruct(BUFFERSIZE);
    char name[MAXLOGINSIZE];
    memset(name, '\0', MAXLOGINSIZE);
    printf("What is your name?\n");
    getInput(buffer);
    int namelen;
    if ((namelen = strlen(buffer -> buffer)) >= LOGIN_NAME_MAX - 1)
    {
        printf("Your name is too long\n");
        return 1;
    }
    strncpy(name, buffer -> buffer, namelen - 1);

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct in_addr addr;
    inet_aton(HOST, &addr);
    struct sockaddr_in sock_in = {AF_INET, htons(PORT), addr, 0};
    if (connect(socketfd, (struct sockaddr *) &sock_in, (socklen_t) sizeof(sock_in)) == -1)
    {
        perror("");
        return 1;
    }

    write(socketfd, name, MAXLOGINSIZE);    //send your name to the server

    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        char file[] = "xterm";
        char* args[] = {"xterm", "-e", "./chat", NULL};
        execvp(file, args);
        exit(3);
    }
    //Buffer* buffer = bufferConstruct(BUFFERSIZE);
    // printf("1\n");
    //receiver part
    pid_t pid = fork();
    if (pid == 0)
    {
        // printf("2\n");
        key_t key = ftok("keyfile.txt", CHAT_KEY);
        int msgid = msgget(key, IPC_CREAT | 0666);
        ChatConnect chatConnect;
        // printf("2\n");
        msgrcv(msgid, &chatConnect, sizeof(chatConnect.n), CHAT_CHANNEL, 0);
        // printf("2\n");
        char fifoName[FIFONAMESIZE];
        sprintf(fifoName, FIFONAMETEMPLATE, chatConnect.n);
        int fifofd = open(fifoName, O_WRONLY);
        // printf("2\n");
        int msgSize;
        while(1)
        {
            msgSize = getMessage(socketfd, buffer);
            write(fifofd, buffer -> buffer, msgSize);
        }

    }
    // printf("1\n");

    //sender part
    int type;
    while(1)
    {
        printf("> ");
        type = getInput(buffer);
        switch(type)
        {
        case TYP_MSG:
            send(socketfd, &type, sizeof(type), 0);
            send(socketfd, buffer -> buffer, strlen(buffer -> buffer), 0);
            break;
        case TYP_EXIT:
            send(socketfd, &type, sizeof(type), 0);
            goto cleanFinish;
        }
        
    }


    cleanFinish:
    bufferDestruct(buffer);
    kill(pid, SIGINT);
    kill(pid1, SIGINT);
    close(socketfd);

    return 0;
}