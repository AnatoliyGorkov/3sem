#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NAMESIZE 128
#define ARGSIZE 16
#define CLOCK_IN_USAGE CLOCK_REALTIME
#define NSEC 1E9
#define USEC 1E6
#define TESTSBUFFER

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }

    int nStart, nEnd, nStep;
    printf("Type in start, end and step for n\n");
    scanf("%d%d%d", &nStart, &nEnd, &nStep);
    int kStart, kEnd, kStep;
    printf("Type in start, end and step for the number of threads\n");
    scanf("%d%d%d", &kStart, &kEnd, &kStep);
    int tnum;   //number of tests
    printf("Type in the number of tests for the same call\n");
    scanf("%d", &tnum);
    FILE* file = fopen(argv[2], "w");
    if (file == NULL)
    {
        fprintf(stderr, "Output file hasn't been opened\n");
    }
    char arg1[ARGSIZE];
    char arg2[ARGSIZE];
    char* parameters[] = {argv[1], arg1, arg2, NULL};
    int n, k, i;
    size_t timeTotal;
    struct timespec start, end;
    pid_t pid;
    int status;
    int testTotal = ((nEnd - nStart) / nStep + 1)*((kEnd - kStart) / kStep + 1);
    int testNumber = 0;
    printf("n = %d Testing %d of %d", n, testNumber, testTotal);
    for (n = nStart; n <= nEnd; n += nStep)
    {
        fprintf(file, "n = %d\n", n);
        fprintf(file, "---------------\n");
        for(k = kStart; k <= kEnd; k += kStep)
        {
            testNumber++;
            printf("\rn = %d Testing %d of %d", n, testNumber, testTotal);
            fseek(stdout, -10, SEEK_CUR);
            timeTotal = 0;
            for(i = 0; i < tnum; i++)
            {
                sprintf(parameters[1], "%d", k);
                sprintf(parameters[2], "%d", n);
                clock_gettime(CLOCK_IN_USAGE, &start);
                pid = fork();
                if (pid == 0)
                {
                    execvp(argv[1], parameters);
                    fprintf(stderr, "Execution error\n");
                    exit(1);
                }
                wait(&status);
                clock_gettime(CLOCK_IN_USAGE, &end);
                timeTotal += (size_t) NSEC * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
            }

            timeTotal /= tnum;
            fprintf(file, "%d\t%lu.%lu\n", k, timeTotal / (size_t) NSEC, (timeTotal % (size_t) NSEC) / (size_t) USEC);

        }
        fprintf(file, "---------------\n");
    }
    printf("\n");
    fclose(file);
    return 0;
}