#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "split.h"

#define BUFFERBLOCKSIZE 1024
#define DELAYPRECISION 1E7  //if changed, output format in printLog must be changed
#define NSECSINSEC 1E9
#define ARGVBLOCKSIZE 16

typedef struct timespec Timespec;

typedef struct Call_ {
    size_t delay;
    char** argv;
} Call;

/* callConstcuct allocates memory for the Call structure,
allocates memory for argv, parses the line with ' ' as delimeter
and fills with tokens call -> argv */

Call* callConstuct(char* line)
{
    if (line == NULL)
        return NULL;
    Call* call = malloc(sizeof(*call));
    if (call == NULL)
        return NULL;
    char* saveptr;
    char delim[] = " ";
    strtok_r(line, delim, &saveptr);
    call -> delay = atoll(line);

    size_t wordCount = 0;
    call -> argv = malloc(sizeof(*(call -> argv)) * ARGVBLOCKSIZE);
    size_t argvSize = ARGVBLOCKSIZE;
    if (call -> argv == NULL)
    {
        free(call);
        return NULL;
    }
    char** callSaveptr;
    do
    {
        if (wordCount >= argvSize)
        {
            argvSize *= 2;
            callSaveptr = realloc(call -> argv, sizeof(*(call -> argv)) * argvSize);
            if (callSaveptr == NULL)
            {
                free(call -> argv);
                free(call);
                return NULL;
            }
            call -> argv = callSaveptr;
        }
        call -> argv[wordCount++] = strtok_r(NULL, delim, &saveptr);
    } while(call -> argv[wordCount - 1] != NULL);
    call -> argv = realloc(call -> argv, wordCount * sizeof(*(call -> argv)));  // cuts the excess of bytes in buffer
    return call;
}

/* callDestruct free all memory allocated for call structure */
int callDestruct(Call* call)
{
    if (call == NULL)
        return -1;
    free(call);
    return 0;
}

/* callComparator is used for qsort function */
int callComparator(const void* a, const void* b)
{
    Call* call1 = *((Call**) a);
    Call* call2 = *((Call**) b);
    return (int) (call1 -> delay) - (int) (call2 -> delay);
}

/* printLog puts in log file information about the executed process*/
int printLog(FILE* logFile, const Timespec* sleepStartTime, const Timespec* execTime, const Call* call)
{
    struct tm parsedTime;
    size_t delayInNsecs = 0, seconds, nanoseconds;
    // if was late to call
    if ((execTime -> tv_sec < sleepStartTime -> tv_sec) ||
        (execTime -> tv_sec == sleepStartTime -> tv_sec && sleepStartTime -> tv_nsec - execTime -> tv_nsec >= DELAYPRECISION))
    {
        delayInNsecs = NSECSINSEC * (sleepStartTime -> tv_sec - execTime -> tv_sec) + sleepStartTime -> tv_nsec - execTime -> tv_nsec;
        seconds = delayInNsecs / NSECSINSEC;
        nanoseconds = delayInNsecs - seconds * NSECSINSEC;
        parsedTime = *(localtime_r(&(sleepStartTime -> tv_sec), &parsedTime));  //clock_nanosleep() hasn't started
    }
    else
        parsedTime = *(localtime_r(&(execTime -> tv_sec), &parsedTime));    //clock_nanosleep() slept till awakeningTime

    fprintf(logFile, "[%02d.%02d.%d  %02d:%02d:%02d] ",
                        parsedTime.tm_mday,
                        parsedTime.tm_mon + 1,
                        parsedTime.tm_year + 1900,
                        parsedTime.tm_hour,
                        parsedTime.tm_min,
                        parsedTime.tm_sec);
    size_t i = 0;
    size_t fractPart = nanoseconds / DELAYPRECISION;
    while (call -> argv[i] != NULL)
        fprintf(logFile, "%s ", call -> argv[i++]);
    if (delayInNsecs != 0)
        fprintf(logFile, "\t[delay: %lu.%02lus]\n", seconds, fractPart);
    else
        fprintf(logFile, "\n");
    fflush(logFile);
    return 0;
}

int main(int argc, char** argv)
{
    Timespec initTime;
    clock_gettime(CLOCK_REALTIME, &initTime);

    if (argc != 2)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }

    int inputFd = open(argv[1], O_RDONLY);
    if (inputFd == -1)
    {
        fprintf(stderr, "Input file hasn't been opened\n");
        return 1;
    }

    char* logfileName = "timeExec.log";
    FILE* logFile = fopen(logfileName, "w");
    if (logFile == NULL)
    {
        fprintf(stderr, "Log file hasn't been created\n");
        close(inputFd);
        return 1;
    }

    size_t bufferSize = BUFFERBLOCKSIZE;
    char* buffer = malloc(sizeof(*buffer) * bufferSize);

    char* bufferCopy = NULL;
    size_t bytesRead, bytesReadTotal = 0;
    do
    {
        if (bytesReadTotal >= bufferSize)
        {
            bufferSize *= 2;
            bufferCopy = buffer;
            buffer = realloc(buffer, sizeof(*buffer) * bufferSize);
            if (buffer == NULL)
            {
                free(bufferCopy);
                close(inputFd);
                fclose(logFile);
                fprintf(stderr, "Error in memory allocation\n");
                return 1;
            }
        }
        bytesRead = read(inputFd, buffer + bytesReadTotal, BUFFERBLOCKSIZE);

        if (bytesRead == -1)
        {
            fprintf(stderr, "Some error occured in file reading\n");
            free(buffer);
            fclose(logFile);
            close(inputFd);
            return 1;
        }

        bytesReadTotal += bytesRead;
    } while (bytesRead == BUFFERBLOCKSIZE);

    buffer = realloc(buffer, bytesReadTotal);   // cuts the excess of bytes in buffer
    close(inputFd);

    size_t numberOfCalls = 0, i = 0;
    while(buffer[i] != '\0' && buffer[i] != EOF)
    {
        if (buffer[i] == '\n')
            numberOfCalls++;
        i++;
    }

    numberOfCalls++;    //to include that last line that may not end with \n
    char** lines = malloc(sizeof(*lines) * numberOfCalls);

    char lineDelimeters[] = {'\n', EOF};
    numberOfCalls = split(buffer, lineDelimeters, lines);

    Call** calls = malloc(sizeof(*calls) * numberOfCalls);
    for(i = 0; i < numberOfCalls; i++)
        calls[i] = callConstuct(lines[i]);

    free(lines);

    qsort(calls, numberOfCalls, sizeof(*calls), callComparator);

    Timespec awakeningTime;
    Timespec sleepStartTime;
    pid_t pid;
    int status;
    for(i = 0; i < numberOfCalls; i++)
    {
        awakeningTime.tv_sec = initTime.tv_sec + calls[i] -> delay;
        awakeningTime.tv_nsec = initTime.tv_nsec;
        clock_gettime(CLOCK_REALTIME, &sleepStartTime);
        clock_nanosleep(CLOCK_REALTIME, TIMER_ABSTIME, &awakeningTime, NULL);
        pid = fork();
        if (pid == 0)
        {
            execvp(calls[i] -> argv[0], calls[i] -> argv);
            fprintf(logFile, "EXECUTION ERROR\t");
            fflush(logFile);
            exit(1);
        }
        wait(&status);
        printLog(logFile, &sleepStartTime, &awakeningTime, calls[i]);
    }

    for(i = 0; i < numberOfCalls; i++)
        callDestruct(calls[i]);

    free(buffer);
    fclose(logFile);
    return 0;
}
