#include "common.h"
#include <stdlib.h>

typedef struct ThreadInfo_
{
    int thread;
    int qid;
    int semid;
    int socket;
    List* onlineConnections;
} ThreadInfo;

typedef struct qvalue_t_
{
    long mtype;
    int freeThread;
} qvalue_t;

int getName(int socket, char* name)
{
    name = memset(name, '\0', MAXLOGINSIZE);
    recv(socket, name, MAXLOGINSIZE, 0);
    return 0;
}

void* threadAction(void* args)
{
    ThreadInfo* threadInfo = (ThreadInfo*) args;
    int socket = threadInfo -> socket;
    struct sembuf a1 = {0, 1, 0};
    struct sembuf d1 = {0, -1, 0};

    semop(threadInfo -> semid, &d1, 1);
    listInsert(threadInfo -> onlineConnections, socket, 1);
    semop(threadInfo -> semid, &a1, 1);
    char name[MAXLOGINSIZE];
    char systemName[MAXLOGINSIZE];
    memset(systemName, '\0', MAXLOGINSIZE);
    strcpy(systemName, "System");
    char leftChatMsg[LEFTCHATMSGSIZE];
    Buffer* buffer = bufferConstruct(BUFFERSIZE);

    qvalue_t qvalue = {QUEUE_CHANNEL, threadInfo -> thread};
    getName(socket, name);
    puts(name);
    int type;
    bool running = true;
    while(running)
    {
        if (recv(socket, &type, sizeof(type), 0) == -1)
        {
            perror("");
            pthread_exit(NULL);
        }

        switch (type)
        {
        case TYP_MSG:
            getMessage(socket, buffer);
            sendMessage(threadInfo -> onlineConnections, buffer -> buffer, name);
            break;
        case TYP_EXIT:
            running = false;
            listRemoveByValue(threadInfo -> onlineConnections, socket);
            sprintf(leftChatMsg, "%s left chat.\n", name);
            sendMessage(threadInfo -> onlineConnections, leftChatMsg, systemName);
            close(socket);
            break;
        }

    }

    msgsnd(threadInfo -> qid, &qvalue, sizeof(qvalue.freeThread), 0);
    pthread_exit(NULL);
}

//printf("hey\n");
//server multiplies 2 numbers from the message on each thread
//cmd line argument specifies the number of threads able to be active at the same time
int main(int argc, char** argv)
{

    if (argc != 3)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }
    long numberOfThreads = atol(argv[1]);
    key_t semkey = ftok("keyfile.txt", SEM_KEY);
    key_t queuekey = ftok("keyfile.txt", QUEUE_KEY);
    int qid = msgget(queuekey, IPC_CREAT | 0666);
    int semid = semget(semkey, 1, IPC_CREAT | 0666);
    struct sembuf a1 = {0, 1, 0};
    semop(semid, &a1, 1);

    ThreadInfo* threadInfo = malloc(numberOfThreads * sizeof(*threadInfo));
    pthread_t* threads = (pthread_t*) malloc(numberOfThreads * sizeof(*threads));
    List* onlineConnections = listConstruct();
    qvalue_t qvalue = {QUEUE_CHANNEL , 0};
    qvalue_t qshvalue = {QUEUE_CHANNEL , 0};
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
    bind(socketfd, (struct sockaddr *) &sock_in, (socklen_t) sizeof(sock_in));
    int maxClients = atoi(argv[2]);
    listen(socketfd, maxClients);
    int socktmp;
    // printf("1\n");
    while(1)
    {
        msgrcv(qid, &qvalue, sizeof(qvalue.freeThread), QUEUE_CHANNEL, 0);

        freeThread = qvalue.freeThread;
        threadInfo[freeThread].qid = qid;
        threadInfo[freeThread].semid = semid;
        threadInfo[freeThread].thread = freeThread;
        threadInfo[freeThread].onlineConnections = onlineConnections;
        threadInfo[freeThread].socket = accept(socketfd, NULL, NULL);
        if (threadInfo[freeThread].socket == -1)
        {
            printf("ACHTUNG\n");
            return 0;
        }
        // errors check
        if (msgrcv(qid, &qshvalue, sizeof(qshvalue.freeThread), SHUTDOWN_CHANNEL, IPC_NOWAIT) != -1)
        {
            msgctl(qid, IPC_RMID, NULL);
            raise(SIGINT);
        }
        //shutdown
        pthread_create(threads + freeThread, NULL, threadAction, threadInfo + freeThread);

    }
    //when there are no active processes the queue is full
    //so when we can receive numberOfThreads - 1
    //(one is already requested when it terminates) messages, all threads finished their work
    printf("Terminating...\n");
    for (i = 0; i < numberOfThreads - 1; i++)
        msgrcv(qid, &qvalue, sizeof(qvalue.freeThread), QUEUE_CHANNEL, 0);

    free(threads);
    free(threadInfo);
    return 0;
}
