#include <stdlib.h>
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#define KEYFILE "table.txt"
#define SEMNUM1 1
#define SEMNUM2 2
#define SEMNUM3 4
#define SHMNUM 3
#define NTYPES 64

int printTime()
{
    struct tm parsedTime;
    struct timespec currentTime;
    clock_gettime(CLOCK_REALTIME, &currentTime);

    parsedTime = *(localtime_r(&(currentTime.tv_sec), &parsedTime));

    printf("[%02d.%02d.%d  %02d:%02d:%02d]",
                        parsedTime.tm_mday,
                        parsedTime.tm_mon + 1,
                        parsedTime.tm_year + 1900,
                        parsedTime.tm_hour,
                        parsedTime.tm_min,
                        parsedTime.tm_sec);
    return 0;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int* getSpeed(char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file == NULL)
        return NULL;
    int speedSize = NTYPES;
    int* speed = (int*) malloc(speedSize * sizeof(*speed));
    int* tmp;
    if (speed == NULL)
    {
        fclose(file);
        return NULL;
    }
    int type, tspeed;

    while(fscanf(file, "%d%d", &type, &tspeed) == 2)
        if (type < speedSize)
            speed[type] = tspeed;
        else
        {
            speedSize = max(2 * speedSize, type);
            tmp = (int*) realloc(speed, sizeof(*speed) * speedSize);
            if (tmp == NULL)
            {
                free(speed);
                fclose(file);
                return NULL;
            }
            speed = tmp;
        }
    fclose(file);
    return speed;
}

void sOp(int semid, int value)
{
    struct sembuf buf = {0, value, 0};
    semop(semid, &buf, 1);
}
