/*
    options:
    <exit> to exit
    <server shutdown> to shut down the server (reqires password that can be set with hashCreator.c)
    (for now the password is password)
    didn't have time to add more functions
    client.c and server.c should be compiled with -lcrypto
    openssl should be installed
*/

#include "common.h"
#include <stdlib.h>

int getInput(Buffer* buffer)
{
    size_t i = 0;
    char c;
    while(1)
    {
        c = getchar();
        if (i >= buffer -> maxSize && bufferExpand(buffer) == NULL)
            return -1;
        *(buffer -> buffer + i) = c;
        i++;
        if (c == '\n')
            break;

    }
    *(buffer -> buffer + i) = '\0';

    if (strcmp(buffer -> buffer, STR_EXIT) == 0)
        return TYP_EXIT;
    if (strcmp(buffer -> buffer, STR_SHTDWN) == 0)
        return TYP_SHTDWN; 
    return TYP_MSG;
}

int getName(Buffer* buffer, char* name)
{
    int namelen;
    bool running = true;
    while(running)
    {
        getInput(buffer);
        if ((namelen = strlen(buffer -> buffer)) > MAXLOGINSIZE)
            printf("Your name is too long. Please type another one\n");
        else
        {
            buffer -> buffer[namelen - 1] = '\0';   //overwrite /n
            running = false;
        }
    }
    strncpy(name, buffer -> buffer, MAXLOGINSIZE);
    return 0;
}

int main(int argc, char** argv)
{
    Buffer* buffer = bufferConstruct(BUFFERSIZE);
    if (buffer == NULL)
    {
        perror("Chat startup error\n");
        return 1;
    }

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    if (socketfd == -1)
    {
        perror("Net problems");
        bufferDestruct(buffer);
        return 1;
    }
    struct in_addr addr;
    inet_aton(HOST, &addr);
    struct sockaddr_in sock_in = {AF_INET, htons(PORT), addr, 0};

    if (connect(socketfd, (struct sockaddr *) &sock_in, (socklen_t) sizeof(sock_in)) == -1)
    {
        perror("Net problems");
        bufferDestruct(buffer);
        close(socketfd);
        return 1;
    }

    // output terminal
    pid_t pid1 = fork();
    if (pid1 == 0)
    {
        char file[] = "xterm";
        char* args[] = {"xterm", "-e", "./chat", NULL};
        execvp(file, args);
        exit(1);
    }

    key_t key = ftok("keyfile.txt", CHAT_KEY);
    int msgid = msgget(key, IPC_CREAT | 0666);
    if (msgid == -1)
    {
        perror("Startup error");
        close(socketfd);
        bufferDestruct(buffer);
        return -1;
    }
    int type;
    int pipefd[2];
    pipe(pipefd);   //used for communication between input and output
    ChatConnect chatConnect;
    msgrcv(msgid, &chatConnect, sizeof(chatConnect.n), CHAT_CHANNEL, 0);
    msgctl(msgid, IPC_RMID, 0);
    char fifoName[FIFONAMESIZE];
    sprintf(fifoName, FIFONAMETEMPLATE, chatConnect.n);
    bool running = true;

    // This part receives messages
    pid_t pid = fork();
    if (pid == 0)
    {
        close(pipefd[0]);
        int fifofd = open(fifoName, O_WRONLY);
        int msgSize;
        int type, serverResponse;
        int errval;
        while(running)
        {
            // recv() returnes 0 is connection has been terminated
            if ((errval = recv(socketfd, &type, sizeof(type), 0)) == -1 || errval == 0)
            {
                running = false;
                break;
            }

            switch (type)
            {
            case TYP_MSG:
                msgSize = getMessage(socketfd, buffer);

                if (msgSize == -1)
                {
                    fprintf(stderr, "Receiving problems\n");
                    running = false;
                    break;
                }
                if (write(fifofd, buffer -> buffer, msgSize) == -1)
                {
                    perror("Lost another window");
                    running = false;
                    break;
                }
                break;
            case TYP_SHTDWN:
                if (recv(socketfd, &serverResponse, sizeof(serverResponse), 0) == -1)
                {
                    fprintf(stderr, "Receiving problems\n");
                    running = false;
                    break;
                }
                if (write(pipefd[1], &serverResponse, sizeof(serverResponse)) == -1)
                {
                    perror("Lost pipe");
                    running = false;
                    break;
                }
                break;
            case TYP_NAMEBUSY:
            case TYP_NAMEOK:
                if (write(pipefd[1], &type, sizeof(type)) == -1)
                {
                    perror("Lost pipe");
                    running = false;
                    break;
                }
                break;
            default:
                break;
            }
        }
        fprintf(stderr, "Server crashed\n");
        kill(pid1, SIGTERM);
        kill(getppid(), SIGTERM);
        if (unlink(fifoName) == -1)
        {
            perror("Unlink");
        }
        return 1;
    }

    //sender part
    close(pipefd[1]);
    char name[MAXLOGINSIZE];
    printf("What is your name?\n");
    getName(buffer, name);
    send(socketfd, name, MAXLOGINSIZE, 0);    //send your name to the server
    read(pipefd[0], &type, sizeof(type));
    while(type != TYP_NAMEOK)
    {
        printf("Sorry, that name is already taken\n");
        printf("Please, type in another one\n");
        getName(buffer, name);
        send(socketfd, name, MAXLOGINSIZE, 0);
        read(pipefd[0], &type, sizeof(type));
    }

    unsigned int serverAnswer;
    unsigned char hash[SHA256_DIGEST_LENGTH];
    while(running)
    {
        printf("> ");
        type = getInput(buffer);
        switch(type)
        {
        case TYP_MSG:
            if (send(socketfd, &type, sizeof(type), 0) == -1 ||
                send(socketfd, buffer -> buffer, strlen(buffer -> buffer), 0) == -1)
            {
                fprintf(stderr, "Server error\n");
                running = false;
                break;
            }
            break;
        case TYP_EXIT:
            if (send(socketfd, &type, sizeof(type), 0) == -1)
            {
                fprintf(stderr, "Server error\n");
                running = false;
                break;
            }
            running = false;
            break;
        case TYP_SHTDWN:
            printf("Type in the password\n");
            while(1)
            {
                echo(false);
                getInput(buffer);
                echo(true);
                SHA256((unsigned char*) buffer -> buffer, strlen(buffer -> buffer) - 1, hash);
                if (send(socketfd, &type, sizeof(type), 0) == -1 ||
                    send(socketfd, hash, SHA256_DIGEST_LENGTH, 0) == -1)
                {
                    fprintf(stderr, "Server error\n");
                    running = false;
                    break;
                }
                read(pipefd[0], &serverAnswer, sizeof(serverAnswer));
                if (serverAnswer == PASWDCONFIRM)
                {
                    running = false;
                    break;
                }
                printf("Wrond password. Try again\n");
            }
            break;
        default:
            fprintf(stderr, "Unknown error\n");
            running = false;
        } 
    }
    bufferDestruct(buffer);
    close(socketfd);
    close(pipefd[0]);
    kill(pid, SIGTERM);
    kill(pid1, SIGTERM);

    if (unlink(fifoName) == -1)
    {
        perror("Unlink");
    }

    return 0;
}
