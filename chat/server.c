#include "common.h"
#include <stdlib.h>

typedef struct ThreadInfo_
{
    int qid;
    int thread;
    int dataSize;
    int semid;
    int socket;
    char* paswdHash;
    Data* users;
} ThreadInfo;

typedef struct QueueValue_t_
{
    long mtype;
    int freeThread;
} QueueValue_t;

// sorry, got tired of copy-paste
// hope for suggestions to make it better
#define THREADFINISH                                                            \
{                                                                               \
    fprintf(stderr, "%s disconnected\n", name);                                 \
    semop(threadInfo -> semid, &d1, 1);                                         \
    users[thread].socket = 0;                                                   \
    users[thread].name[0] = '\0';                                               \
    semop(threadInfo -> semid, &a1, 1);                                         \
    bufferDestruct(buffer);                                                     \
    msgsnd(threadInfo -> qid, &qvalue, sizeof(qvalue.freeThread), 0);           \
    close(socket);                                                              \
    sprintf(disconnectedMsg, "%s disconnected.\n", name);                       \
    sendMessage(users, dataSize, thread, disconnectedMsg, systemName, TYP_MSG); \
    pthread_exit(NULL);                                                         \
}

long findName(Data* users, size_t size, char* name)
{
    size_t i;
    for(i = 0; i < size; i++)
    {
        if (users[i].socket != 0)
            if (strncmp(name, users[i].name, MAXLOGINSIZE) == 0)
                return i;
    }
    return -1;
}

int getName(int socket, char* name, Data* users, size_t size)
{
    int type;
    int retval;
    bool nameAccepted = false;
    while(!nameAccepted)
    {
        retval = recv(socket, name, MAXLOGINSIZE, 0);
        if (retval == -1 || retval == 0)
        {
            fprintf(stderr, "Name is not received\n");
            return -1;
        }

        if (findName(users, size, name) == -1)
        {
            nameAccepted = true;
            type = TYP_NAMEOK;
        }
        else
        {
            type = TYP_NAMEBUSY;
        }

        if (send(socket, &type, sizeof(type), 0) == -1)
        {
            fprintf(stderr, "Name is not sent\n");
            return -1;
        }
    }
    
    return 0;
}

int closeAllConnections(Data* users, size_t size)
{
    size_t i;
    for (i = 0; i < size; i++)
        if (users[i].socket != 0)
            close(users[i].socket);
    return 0;
}


void* threadAction(void* args)
{
    ThreadInfo* threadInfo = (ThreadInfo*) args;
    int socket = threadInfo -> socket;
    Data* users = threadInfo -> users;
    int thread = threadInfo -> thread;
    int dataSize = threadInfo -> dataSize;
    struct sembuf a1 = {0, 1, 0};
    struct sembuf d1 = {0, -1, 0};
    QueueValue_t qvalue = {QUEUE_CHANNEL, threadInfo -> thread};

    semop(threadInfo -> semid, &d1, 1);
    users[thread].socket = socket;
    semop(threadInfo -> semid, &a1, 1);
    char systemName[MAXLOGINSIZE];
    strcpy(systemName, "System");
    char name[MAXLOGINSIZE];
    char leftChatMsg[LEFTCHATMSGSIZE];
    char disconnectedMsg[LEFTCHATMSGSIZE];
    Buffer* buffer = bufferConstruct(BUFFERSIZE);
    int type, fd, responseType;
    if (getName(socket, name, users, dataSize) == -1)
    {
        THREADFINISH;
    }
    strncpy(users[thread].name, name, MAXLOGINSIZE);
    printf("%s connected\n", users[thread].name);

    unsigned int paswdResult;
    int retval;
    bool running = true;
    while(running)
    {
        if ((retval = recv(socket, &type, sizeof(type), 0)) == -1 || retval == 0)
        {
            fprintf(stderr, "Connection lost\n");
            THREADFINISH;
        }

        switch (type)
        {
        case TYP_MSG:
            if (getMessage(socket, buffer) == -1)
            {
                fprintf(stderr, "Message not accepted\n");
                THREADFINISH;
            }
            if (sendMessage(users, dataSize, thread, buffer -> buffer, name, TYP_MSG) == -1) //if all ok
            {
                fprintf(stderr, "%s disconnected\n", users[thread].name);
                THREADFINISH;
            }
            break;

        case TYP_EXIT:
            running = false;

            semop(threadInfo -> semid, &d1, 1);
            users[thread].socket = 0;
            users[thread].name[0] = '\0';
            semop(threadInfo -> semid, &a1, 1);

            sprintf(leftChatMsg, "%s left chat.\n", name);
            if (sendMessage(users, dataSize, thread, leftChatMsg, systemName, TYP_MSG) == -1)    //if all ok
            {
                fprintf(stderr, "%s disconnected\n", users[thread].name);
                THREADFINISH;
            }
            close(socket);
            break;

        case TYP_SHTDWN:
            if ((retval = recv(socket, buffer -> buffer, SHA256_DIGEST_LENGTH, 0)) == -1 || retval == 0)
            {
                fprintf(stderr, "Password not accepted\n");
                THREADFINISH;
            }
            if (strncmp(buffer -> buffer, threadInfo -> paswdHash, SHA256_DIGEST_LENGTH))
                paswdResult = PASWDWROND;
            else
                paswdResult = PASWDCONFIRM;
            responseType = TYP_SHTDWN;
            sleep(1);
            if (send(socket, &responseType, sizeof(responseType), 0) == -1 ||
                send(socket, &paswdResult, sizeof(paswdResult), 0) == -1)
            {
                fprintf(stderr, "Password result not sent\n");
                THREADFINISH;
            }
            if (paswdResult == PASWDCONFIRM)
            {
                fprintf(stderr, "Termination signal accepted\n");
                semop(threadInfo -> semid, &d1, 1);
                users[thread].socket = 0;
                users[thread].name[0] = '\0'; 
                semop(threadInfo -> semid, &a1, 1);
                closeAllConnections(users, dataSize);
                close(socket);
                bufferDestruct(buffer);
                semctl(threadInfo -> semid, IPC_RMID, 0);
                msgctl(threadInfo -> qid, IPC_RMID, 0);
                exit(1);
            }
            break;
        }

    }

    bufferDestruct(buffer);
    //msgsnd(threadInfo -> qid, &qvalue, sizeof(qvalue.freeThread), 0);
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }
    char paswdHash[SHA256_DIGEST_LENGTH];
    int fd = open(PASWDFILE, O_RDONLY);
    if (fd == -1)
    {
        perror("fd");
        return 1;
    }
    read(fd, paswdHash, SHA256_DIGEST_LENGTH);
    close(fd);
    long numberOfThreads = atol(argv[1]);
    ThreadInfo* threadInfo = malloc(numberOfThreads * sizeof(*threadInfo));
    if (threadInfo == NULL)
    {
        perror("Allocation");
        return 1;
    }
    pthread_t* threads = (pthread_t*) malloc(numberOfThreads * sizeof(*threads));
    if (threads == NULL)
    {
        perror("Allocation");
        return 1;
    }
    Data* users = (Data*) calloc(numberOfThreads, sizeof(Data));
    if (users == NULL)
    {
        perror("Allocation");
        return 1;
    }

    key_t semkey = ftok("keyfile.txt", SEM_KEY);
    key_t queuekey = ftok("keyfile.txt", QUEUE_KEY);
    int qid = msgget(queuekey, IPC_CREAT | 0666);
    int semid = semget(semkey, 1, IPC_CREAT | 0666);
    struct sembuf a1 = {0, 1, 0};
    semop(semid, &a1, 1);
    QueueValue_t qvalue = {QUEUE_CHANNEL , 0};
    int i;
    for (i = 0; i < numberOfThreads; i++)
    {
        qvalue.mtype = QUEUE_CHANNEL;
        qvalue.freeThread = i;
        msgsnd(qid, &qvalue, sizeof(qvalue.freeThread), 0);
    }
    int freeThread;

    int socketfd = socket(AF_INET, SOCK_STREAM, 0);
    struct in_addr addr;
    inet_aton(HOST, &addr);
    struct sockaddr_in sock_in = {AF_INET, htons(PORT), addr, 0};
    if (bind(socketfd, (struct sockaddr *) &sock_in, (socklen_t) sizeof(sock_in)) == -1)
    {
        perror("bind: ");
        return 1;
    }

    if (listen(socketfd, numberOfThreads) == -1)
    {
        perror("listen: ");
        return 1;
    }

    printf("Started accepting messages\n");
    while(1)
    {
        if (msgrcv(qid, &qvalue, sizeof(qvalue.freeThread), QUEUE_CHANNEL, 0) == -1)
        {
            perror("msg queue");
            semctl(semid, IPC_RMID, 0);
            return 1;
        }
        freeThread = qvalue.freeThread;
        threadInfo[freeThread].qid = qid;
        threadInfo[freeThread].semid = semid;
        threadInfo[freeThread].dataSize = numberOfThreads;
        threadInfo[freeThread].users = users;
        threadInfo[freeThread].paswdHash = paswdHash;
        threadInfo[freeThread].thread = freeThread;
        threadInfo[freeThread].socket = accept(socketfd, NULL, NULL);

        if (threadInfo[freeThread].socket == -1)
        {
            printf("ACHTUNG\n");
            return 1;
        }
        pthread_create(threads + freeThread, NULL, threadAction, threadInfo + freeThread);
    }

    free(threads);
    free(threadInfo);
    return 0;
}
