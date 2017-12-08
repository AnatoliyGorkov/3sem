#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/sem.h>
#include <pthread.h>
#include <sys/msg.h>
#include <unistd.h>
#include <openssl/sha.h>
#include <termios.h>
#include <stdbool.h>
#include <signal.h>
#include "list.h"
#include <string.h>
#include <errno.h>
typedef int int32_t;

#define CHAT_KEY 5
#define QUEUE_CHANNEL 2
#define SHUTDOWN_CHANNEL 3
#define CHAT_CHANNEL 9
#define QUEUE_KEY 2
#define SEM_KEY 3
#define PORT 58432
#define HOST "127.0.0.1"
#define FRAMESIZE 64
#define MAXLOGINSIZE 32
#define ENDMESSAGE 0xDEADBEEF
#define BUFFERSIZE 1024
#define FIFONAMESIZE 32
#define FIFONAMETEMPLATE "chat%d.fifo"
#define LEFTCHATMSGSIZE 64
#define TYP_MSG 0xaaaaaaaa
#define TYP_EXIT 0x55555555

#define STR_EXIT "<exit>\n"

typedef struct ChatConnect_
{
    long mtype;
    int n;
} ChatConnect;

typedef struct Buffer_
{
    char* buffer;
    size_t maxSize;
} Buffer;

void echo(bool mode) //0 to disable
{
    struct termios settings;
    tcgetattr(STDIN_FILENO, &settings);
    if (mode)
        settings.c_lflag |= ECHO;
    else
        settings.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &settings);
}

Buffer* bufferConstruct(size_t maxSize)
{
    Buffer* buffer = (Buffer*) malloc(sizeof(*buffer));
    char* data = (char*) malloc(maxSize * sizeof(*data));
    buffer -> buffer = data;
    buffer -> maxSize = maxSize;
    return buffer;
}

int bufferDestruct(Buffer* buffer)
{
    free(buffer -> buffer);
    free(buffer);
    return 0;
}

Buffer* bufferExpand(Buffer* buffer)
{
    char* tmp = (char*) realloc(buffer -> buffer, 2 * buffer -> maxSize * sizeof(char));
    if (tmp == NULL)
    {
        tmp = (char*) realloc(buffer -> buffer, sizeof(char) * (buffer -> maxSize + FRAMESIZE));
        if (tmp == NULL)
            return NULL;
        buffer -> buffer = tmp;
        buffer -> maxSize += FRAMESIZE;
        return buffer;
    }
    buffer -> buffer = tmp;
    buffer -> maxSize *= 2;
    return buffer;
}

int getMessage(int socket, Buffer* buffer)
{
    int bytesRead;
    int bytesReadTotal = 0;
    Buffer* tmp;
    while(1)
    {
        // printf("2\n");
        bytesRead = read(socket, buffer -> buffer + bytesReadTotal, FRAMESIZE);
        if (bytesRead == -1)
        {
            perror("wtf");
        }
        // printf("3\n");
        // printf("%d\n", bytesRead);
        if (bytesRead < 1)
            break;
        bytesReadTotal += bytesRead;
        if (*(buffer -> buffer + bytesReadTotal - 1) == '\n')
            break;
        if (bytesReadTotal + FRAMESIZE >= buffer -> maxSize)
        {
            tmp = bufferExpand(buffer);
            if (tmp != NULL)
                buffer = tmp;
        }
    }
    *(buffer -> buffer + bytesReadTotal) = '\0';
    // printf("<>%d\n", bytesReadTotal);
    return bytesReadTotal;
}

int sendMessage(List* users, char* message, char* name)
{
    if (users -> size == 0)
        return 1;
    ListElement* elem = users -> head;
    int i;
    for (i = 0; i < users -> size; i++)
    {
        if (send(elem -> value, name, MAXLOGINSIZE, 0) == -1)
            perror("send1");
        if (send(elem -> value, message, strlen(message), 0) == -1)
            perror("send2");
        elem = elem -> next;
    }
    return 0;
}