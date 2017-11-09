#include "common.h"
#include <stdlib.h>

typedef struct ThreadInfo_
{
    Request request;
    int thread;
    int msgid;
    int qid;
} ThreadInfo;

typedef struct qvalue_t_
{
    long mtype;
    int freeThread;
} qvalue_t;

void* threadMult(void* args)
{
    ThreadInfo* threadInfo = (ThreadInfo*) args;
    Answer answer;

    answer.mtype = (threadInfo -> request).data.callerId;
    answer.data.c = (threadInfo -> request).data.a * (threadInfo -> request).data.b;
    msgsnd(threadInfo -> msgid, &answer, sizeof(answer.data), 0);

    qvalue_t qvalue = {QUEUE_CHANNEL, threadInfo -> thread};
    sleep(15);
    msgsnd(threadInfo -> qid, &qvalue, sizeof(qvalue.freeThread), 0);
    pthread_exit(NULL);
}
//printf("hey\n");
//server multiplies 2 numbers from the message on each thread
//cmd line argument specifies the number of threads able to be active at the same time
int main(int argc, char** argv)
{

    if (argc != 2)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }
    long numberOfThreads = atol(argv[1]);
    key_t msgkey = ftok("keyfile.txt", MSG_KEY);
    key_t queuekey = ftok("keyfile.txt", QUEUE_KEY);

    int msgid = msgget(msgkey, IPC_CREAT | 0666);
    int qid = msgget(queuekey, IPC_CREAT | 0666);

    ThreadInfo* threadInfo = malloc(numberOfThreads * sizeof(*threadInfo));
    pthread_t* threads = (pthread_t*) malloc(numberOfThreads * sizeof(*threads));
    qvalue_t qvalue = {QUEUE_CHANNEL , 0};
    int i;
    for (i = 0; i < numberOfThreads; i++)
    {
        qvalue.mtype = QUEUE_CHANNEL;
        qvalue.freeThread = i;
        msgsnd(qid, &qvalue, sizeof(qvalue.freeThread), 0);
    }

    int freeThread;

    while(1)
    {
        msgrcv(qid, &qvalue, sizeof(qvalue.freeThread), QUEUE_CHANNEL, 0);
        freeThread = qvalue.freeThread;

        msgrcv(msgid, &(threadInfo[freeThread].request), sizeof(threadInfo[freeThread].request.data), INITIAL_CHANNEL, 0);

        if (threadInfo[freeThread].request.data.callerId == 1)
        {
            printf("Termination signal accepted\n");
            break;
        }

        threadInfo[freeThread].msgid = msgid;
        threadInfo[freeThread].qid = qid;
        threadInfo[freeThread].thread = freeThread;

        pthread_create(threads + freeThread, NULL, threadMult, threadInfo + freeThread);

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
