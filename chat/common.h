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
#include <string.h>
#include <errno.h>
typedef int int32_t;
typedef unsigned int uint32_t;
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
#define TYP_NAMEOK 0x2222222
#define TYP_NAMEBUSY 0x1234567
#define TYP_MSG 0xaaaaaaaa
#define TYP_EXIT 0x55555555
#define TYP_SHTDWN 0x11111111   //server shutdown command
#define STR_EXIT "<exit>\n"
#define STR_SHTDWN "<server shutdown>\n"
#define PASWDCONFIRM 0xCCCCCCCC
#define PASWDWROND 0xDEADBEEF
#define PASWDFILE "shadow.txt"

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

typedef struct Data_
{
    int socket;
    int friendsSocket;
    char name[MAXLOGINSIZE];
} Data;

void echo(bool mode) //mode = false to disable input to terminal
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
    if (buffer == NULL)
        return NULL;
    char* data = (char*) malloc(maxSize * sizeof(*data));
    if (data == NULL)
        return NULL;
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

/*
    bufferExpand doubles the size of the buffer
    if it can't if increaces the size by FRAMESIZE bytes
    if it can't - fails with NULL
*/
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

/*
    -1 on error
*/
int getMessage(int socket, Buffer* buffer)
{
    int bytesRead;
    int bytesReadTotal = 0;
    while(1)
    {
        bytesRead = recv(socket, buffer -> buffer + bytesReadTotal, FRAMESIZE, 0);
        if (bytesRead == -1 || bytesRead == 0)
            return -1;
        if (bytesRead == 0)
            break;
        bytesReadTotal += bytesRead;
        if (*(buffer -> buffer + bytesReadTotal - 1) == '\n')
            break;
        if (bytesReadTotal + FRAMESIZE >= buffer -> maxSize)
        {
            if (bufferExpand(buffer) == NULL)
                return -1;
        }
    }
    *(buffer -> buffer + bytesReadTotal) = '\0';
    return bytesReadTotal;
}

int getFifoMessage(int fd, Buffer* buffer)
{
    int bytesRead;
    int bytesReadTotal = 0;
    while(1)
    {
        bytesRead = read(fd, buffer -> buffer + bytesReadTotal, FRAMESIZE);
        if (bytesRead == -1)
            return -1;
        if (bytesRead == 0)
            break;
        bytesReadTotal += bytesRead;
        if (*(buffer -> buffer + bytesReadTotal - 1) == '\n')
            break;
        if (bytesReadTotal + FRAMESIZE >= buffer -> maxSize)
        {
            if (bufferExpand(buffer) == NULL)
                return -1;
        }
    }
    *(buffer -> buffer + bytesReadTotal) = '\0';
    return bytesReadTotal;
}

/*
    if return value is not 0, error occured. Errors if thread == damaged thread. Errno is set by call to send()
*/
int sendMessage(Data* users, size_t size, int thread, const char* message, const char* name, int msgType)
{
    int socketDamaged = 0;
    size_t i = 0;
    switch(msgType)
    {
    case TYP_MSG:
        for (i = 0; i < size; i++)
        {
            if (users[i].socket != 0)
            {
                if ((send(users[i].socket, &msgType, sizeof(msgType), 0) == -1 ||
                    send(users[i].socket, name, MAXLOGINSIZE, 0) == -1 ||
                    send(users[i].socket, message, strlen(message), 0) == -1)
                    && i == thread)
                    socketDamaged = -1;
            }
        }
    }
    return socketDamaged;
}
