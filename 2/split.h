#ifndef SPLIT_USAGE

#include <stdlib.h>
#include <string.h>
#define MAX_DELIM_SIZE 1000

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
    
    // Так намного лучше. Ещё можно переименовать i в tokensCount или что-то вроде
    // Плюс кажется, что можно избавиться от строки *tokens = curr_word; ,
    // заменив цикл while на do-while. Ещё вам точно больше нравится gnu style,
    // когда слова всегда пишутся со строчной буквы и разделяются подчеркиванием?
    // currentWord меньше нравится, чем curr_word? Не настаиваю, дело вкуса.
    return i;
}

#define SPLIT_USAGE
#endif

