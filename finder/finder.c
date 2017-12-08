#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>
#include "path.h"
#define PATHSIZE 64
#define MAX_DNAME_SIZE 256

/*
заменить 1, 0, -1 на enum
*/
enum Result {
	Found,
	NotFound,
	Error
};

/*
можно обойтись без дополнительной  currentLevel
*/
Result finderRecursive(char* name, Path* path, int depth, int currentLevel)    //1 if found, 0 if not, -1 if error
{
    if (currentLevel > depth)
        return NotFound;
    if (path == NULL || path -> name == NULL)
        return Error;
    int retval;
    DIR* dir = opendir(path -> name);
    if (dir == NULL)
        return Error;
    struct dirent* dirent;
    while((dirent = readdir(dir)) != NULL)
    {
        if (dirent -> d_type == DT_REG)
            if (strncmp(name, dirent -> d_name, MAX_DNAME_SIZE) == 0)
                return 1;
    }
    rewinddir(dir);
    while((dirent = readdir(dir)) != NULL)
    {
        if (dirent -> d_type == DT_DIR && 
            strncmp(".", dirent -> d_name, 1) && 
            strncmp("..", dirent -> d_name, 2))
            {
                pathAdd(path, dirent -> d_name);
                if ((retval = finderRecursive(name, path, depth, currentLevel + 1)) == 1)
                    return 1;
                else if (retval == 0)
                    pathCutLevel(path);
                else
                    return -1;
            }
    }
    return 0;
}

int findFile(char* name, int depth)
{
    int retval;
    Path* path = pathConstruct(PATHSIZE);
    if (path == NULL) 
    {
        fprintf(stderr, "Execution error\n");
        return 1;
    }
    path -> name [0] = '.';
    path -> name[1] = '\0';
    path -> currSize++;
    if ((retval = finderRecursive(name, path, depth, 0)) == 0)
        printf("File has not been found\n");
    else if (retval == 1)
        printf("%s\n", path -> name);
    else
        perror("Execution error ");
    pathDestruct(path);
    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect input parameters\n");
        return 1;
    }

    char* endptr;
    int depth = strtol(argv[2], &endptr, 10);
    if (*endptr != '\0')
    {
        fprintf(stderr, "Invalid number format\n");
        return 1;
    }

    char* name = argv[1];
    findFile(name, depth);
    return 0;
}
