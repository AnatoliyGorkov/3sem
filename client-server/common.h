#include <sys/sem.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>
#define INITIAL_CHANNEL 1
#define QUEUE_CHANNEL 2
#define TERMINATE_VALUE 666
#define MSG_KEY 1
#define QUEUE_KEY 2

typedef struct Request_
{
    long mtype;
    struct
    {
        long a;
        long b;
        pid_t callerId;
    } data;
} Request;

typedef struct Answer_
{
    long mtype;
    struct
    {
        long c;
    } data;
} Answer;

