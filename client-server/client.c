#include "common.h"
#include <stdlib.h>

//run with a parameter 666 to cause server shutdown
int main(int argc, char** argv)
{
    key_t key = ftok("keyfile.txt", MSG_KEY);
    int msgid = msgget(key, 0666);
    if (msgid == -1)
    {
        fprintf(stderr, "The server is on maintenance. Please try later\n");
        return 1;
    }
    Answer answer;
    long a = 0, b = 0;
    pid_t pid;
    Request request;

    if (argc == 2)
    {
        if (atoi(argv[1]) != TERMINATE_VALUE)
            fprintf(stderr, "Incorrect termination sequence\n");
        else
        {
            request.mtype = INITIAL_CHANNEL;
            request.data.a = 0;
            request.data.b = 0;
            request.data.callerId = 1;
            msgsnd(msgid, &request, sizeof(request.data), 0);
            printf("Termination signal has been sent to the server\n");
        }

        return 0;
    }
    else
    {
        pid = getpid();
        if (scanf("%ld%ld", &a, &b) != 2)
        {
            fprintf(stderr, "Incorrect input\n");
            return 1;
        }
    }        

    request.mtype = INITIAL_CHANNEL;
    request.data.a = a;
    request.data.b = b;
    request.data.callerId = pid;
    msgsnd(msgid, &request, sizeof(request.data), 0);
    msgrcv(msgid, &answer, sizeof(answer.data), pid, 0);
    printf("%ld\n", answer.data.c);
    return 0;
}