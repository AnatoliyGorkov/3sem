#ifndef SPLIT_USAGE

#include <stdlib.h>
#define MAX_DELIM_SIZE 1000

int delimetersContain(const char*, char, size_t);
int split(char*, const char*, char**);

int delimetersContain(const char* delimeters, char c, size_t n)    // tries no more than n delimeter characters
{
    if (delimeters == NULL)
        return -1;
    while (*delimeters != '\0' && *delimeters != c)
        delimeters++;
    if (*delimeters != '\0')
        return 1;
    else
        return 0;
}

int split(char* string, const char* delimeters, char** tokens)
{
    if (string == NULL || delimeters == NULL || tokens == NULL)
        return -1;
    int i = 0, j = 0;

    if (!delimetersContain(delimeters, *string, MAX_DELIM_SIZE))
    {
        *tokens = string;
        j++;
        i++;
    }

    while (string[i])
    {
        if (delimetersContain(delimeters, string[i], MAX_DELIM_SIZE))
            string[i] = '\0';
        else if (string[i - 1] == '\0')
        {
            tokens[j] = string + i;
            j++;
        }
        i++;
    }

    return j;
}

#define SPLIT_USAGE
#endif

