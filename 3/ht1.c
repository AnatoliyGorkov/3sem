#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct ProcessInfo_
{
    pid_t ppid;
    pid_t pid;
} ProcessInfo;

int searchForOffspring(int* tree, int processNumber);    // searches for one of the kids
{
    int count;
    for (count = 1; count <=  )
    return 0;
}
int recursiveFork(int processNumber, int* tree, ProcessInfo* processInfo)   // proseccNumber is the number of the current process
{
    processInfo[processNumber].ppid = getppid();
    processInfo[processNumber].pid = getpid();


    return 0;
}

int main()
{
    size_t numberOfProcesses;
    int i;
    scanf("%lu", &numberOfProcesses);
    int* tree = malloc(sizeof(*tree) * (numberOfProcesses + 2));
    ProcessInfo* processInfo = malloc((numberOfProcesses + 1) * sizeof(*processInfo));
    getTree(tree, numberOfProcesses);
    tree[numberOfProcesses + 1] = -1;


    for (i = 1; i <= numberOfProcesses; i++)
        scanf("%d", tree + i);




    recursiveFork(0, tree, processInfo);





    return 0;
}