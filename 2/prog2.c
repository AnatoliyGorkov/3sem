#include "split.h"
#include <stdio.h>
#include <stdlib.h>

#define STRING_SIZE 1000


int main()
{
    char* string = malloc(STRING_SIZE * sizeof(*string));
    char** tokens = malloc(STRING_SIZE * sizeof(*tokens));
    fgets(string, STRING_SIZE, stdin);
    char delimeters[] = {'\t', ' ', ',', '.', '\n'};
    int token_num = split(string, delimeters, tokens);
    printf("%d\n", token_num);
    int i;
    for(i = 0; i < token_num; i++)
        puts(tokens[i]);
    free(string);
    free(tokens);
    return 0;
}

