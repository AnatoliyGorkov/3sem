#ifndef PATH_USAGE

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct Path_
{
    size_t maxSize;
    size_t currSize;
    char* name;
} Path;

Path* pathConstruct(size_t size)
{
    Path* path = (Path*) malloc(sizeof(*path));
    if (path == NULL)
        return NULL;
    path -> name = (char*) malloc(sizeof(*path) * (size + 1));
    if (path -> name == NULL)
    {
        free(path);
        return NULL;
    }
    path -> currSize = 0;
    path -> maxSize = size;
    return path;
}

int pathDestruct(Path* path)
{
    if (path == NULL)
        return 1;
    if (path -> name == NULL)
    {
        free(path);
        return 0;
    }
    free(path -> name);
    free(path);
    return 0;
}

int pathEnlarge(Path* path, size_t size)    //increases max path size on no less than size characters
{
    char* tmp;

    if (path -> maxSize <= size)
    {
        tmp = (char*) realloc(path -> name, sizeof(char) * (size + path -> maxSize + 1));
        if (tmp == NULL)
            return 1;
        path -> maxSize += size;
        path -> name = tmp;
        return 0;
    }

    tmp = (char*) realloc(path -> name, sizeof(char) * (2 * path -> maxSize + 1));

    if (tmp == NULL)
    {
        tmp = (char*) realloc(path -> name, sizeof(char) * (size + path -> maxSize + 1));
        path -> maxSize +=size;
    } 
    else
        path -> maxSize *= 2;

    if (tmp == NULL)
        return 1;
    path -> name = tmp;
    return 0;
}

int pathAdd(Path* path, char* fill) //ads a directory to pathname
{
    if (path == NULL || path -> name == NULL)
        return 1;
    size_t fillSize = strlen(fill);
    if (path -> currSize + fillSize > path -> maxSize)
        if (pathEnlarge(path, fillSize)) 
            return 1;
    sprintf(path -> name + path -> currSize, "/%s", fill);
    path -> currSize += fillSize + 1;
    return 0;
}

int pathCutLevel(Path* path)    //cuts path till the previous '/' (including '/')
{
    if (path == NULL || path -> name == NULL || path -> currSize == 0)
        return 1;
    path -> currSize--;
    while(path -> currSize != 0 && path -> name [path -> currSize] != '/')
    {
        path -> name [path -> currSize] = '\0';
        path -> currSize--;
    }
    if (path -> currSize == 0)
        return 1;
    path -> name [path -> currSize] = '\0';
    return 0;
}

#define PATH_USAGE 1

#endif
