#ifndef SPLIT_USAGE

#include <stdlib.h>
#include <string.h>

int split(char* string, const char* delimeters, char** tokens)
{
    char* saveptr;
    char* curr_word;
    curr_word = strtok_r(string, delimeters, &saveptr);
    if (curr_word == NULL)
        return 0;
    *tokens = curr_word;
    int i = 1;
    while((curr_word = strtok_r(NULL, delimeters, &saveptr)) != NULL)
        tokens[i++] = curr_word;
    return i;
}

#define SPLIT_USAGE
#endif

