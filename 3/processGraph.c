#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define ENDTREE -2
#define OFFSPRINGCHECKED -1

int* getTree(size_t numberOfProcesses)
{
    int* tree = malloc(sizeof(*tree) * (numberOfProcesses + 2));
    if (tree == NULL)
        return NULL;
    tree[0] = -1 * getpid();
    tree[numberOfProcesses + 1] = ENDTREE;
    uint i;
    for (i = 1; i <= numberOfProcesses; i++)
        if (scanf("%d", tree + i) != 1)
        {
            fprintf(stderr, "Incorrect input\n");
            free(tree);
            return NULL;
        }
    return tree;
}

int searchForOffspring(int* tree, int processNumber)    // searches for one of the kids
{
    int count = 1;
    if (tree == NULL)
        return -1;
    while (tree[count] != processNumber && tree[count] != ENDTREE)
        count++;
    if (tree[count] == ENDTREE)
        return 0;
    tree[count] = OFFSPRINGCHECKED;
    return count;
}

int recursiveFork(int processNumber, int* tree, FILE* file)   // proseccNumber is the number of the current process
{
    if (tree == NULL || file == NULL)
        return -1;
    int offspringNumber;
    pid_t pid;
    int status;
    while ((offspringNumber = searchForOffspring(tree, processNumber)) != 0)
    {
        pid = fork();
        if (pid == 0)
        {
            fprintf(file, "\tp%d [label=\"%d\\npid: %d\"]\n", offspringNumber, offspringNumber, getpid());
            fprintf(file, "\tp%d -> p%d\n", processNumber, offspringNumber);
            printf("%d created %d\n", getppid(), getpid());
            fflush(NULL);
            recursiveFork(offspringNumber, tree, file);
            printf("%d out\n", getpid());
            fflush(stdout);
            free(tree);
            fclose(file);
            exit(0);
        }
        wait(&status);
    }
    return 0;
}

int drawTree(char* filename)    //for this function graphviz must be installed
{
    if (filename == NULL)
        return -1;
    //dot file compiling
    int status;
    pid_t pid = fork();
    if (pid == 0)
    {
        char* dotProgram = "dot";
        char* dotArguments[] = {"dot", "-Tpng", "-O", filename, NULL};
        execvp(dotProgram, dotArguments);
        fprintf(stderr, "Problems in dot execution\n");
        exit(1);
    }
    wait(&status);
    //dot file removal
    pid = fork();
    if (pid == 0)
    {
        char* rmProgram = "rm";
        char* rmArguments[] = {"rm", filename, NULL};
        execvp(rmProgram, rmArguments);
        fprintf(stderr, "Problems in dot file removal\n");
        exit(1);
    }
    wait(&status);

    return 0;
}
int main()
{
    // Initial process number is taken zero
    size_t numberOfProcesses;
    if (scanf("%lu", &numberOfProcesses) != 1)
    {
        fprintf(stderr, "Incorrect input\n");
        return 1;
    }
    int* tree = getTree(numberOfProcesses);  // !!!memory for a tree is allocated by malloc, so it must be freed
    if (tree == NULL)
    {
        fprintf(stderr, "Error in tree's memory allocation\n");
        return 1;
    }
    char filename[] = "treeByDot";
    FILE* file = fopen(filename, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Can't open a file for writing\n");
        return 1;
    }
    fprintf(file, "digraph tree {\n");
    fprintf(file, "\tp0 [label=\"0\\npid: %d\"]\n", getpid());
    fflush(NULL);
    
    recursiveFork(0, tree, file);
    fprintf(file, "}\n");
    fclose(file);
    drawTree(filename);

    free(tree);
    return 0;
}