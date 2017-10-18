#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define MAXMSGSIZE 1024

typedef struct params_t_ {
    int user;
    char* fifo_name;
    char* buffer;
} params_t;

void* msg_out(void* args)
{
    params_t* out = (params_t*) args;
    int outFd = open(out -> fifo_name, O_WRONLY);
    if (outFd < 0)
    {
        fprintf(stderr, "outFd error\n");
        pthread_exit(NULL);
    }

    while(1)
    {
        fgets(out -> buffer, MAXMSGSIZE, stdin);
        write(outFd, out -> buffer, MAXMSGSIZE);
        // printf("%d\n", write(outFd, outBuffer, MAXMSGSIZE));
    }
    close(outFd);
    pthread_exit(NULL);
}

void* msg_in(void* args)
{
    params_t* in = (params_t*) args;
    int inFd = open(in -> fifo_name, O_RDONLY);
    if (inFd < 0)
    {
        fprintf(stderr, "inFd error\n");
        pthread_exit(NULL);
    }
    while(read(inFd, in -> buffer, MAXMSGSIZE) > 0)
    {
        // printf("read: %d\n", read(inFd, inBuffer, MAXMSGSIZE));
        printf("< %s", in -> buffer);
    }
    close(inFd);
    pthread_exit(NULL);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Incorrect cmd line parameters\n");
        return 0;
    }

    int clientNumber = atoi(argv[1]);
    int receiverNumber = 1 - clientNumber;

    char* fifoName[] = {"message0.fifo", "message1.fifo"};

    mknod(fifoName[clientNumber], S_IFIFO | 0666, 0);
    mknod(fifoName[receiverNumber], S_IFIFO | 0666, 0);

    char* inBuffer = malloc(sizeof(*inBuffer) * MAXMSGSIZE);
    char* outBuffer = malloc(sizeof(*outBuffer) * MAXMSGSIZE);

    pthread_t thread_in, thread_out;
    params_t args_in = {clientNumber, fifoName[clientNumber], inBuffer};
    params_t args_out = {receiverNumber, fifoName[receiverNumber], outBuffer};

    pthread_create(&thread_in, NULL, msg_in, &args_in);
    pthread_create(&thread_out, NULL, msg_out, &args_out);

    pthread_join(thread_in, NULL);
    pthread_join(thread_out, NULL);

    free(inBuffer);
    free(outBuffer);

    return 0;
}