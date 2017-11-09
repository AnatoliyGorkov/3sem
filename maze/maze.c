//maze must be rectangular
typedef int qvalue_t;
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* getMaze(FILE* input, size_t* width, size_t* height)
{
    if (input == NULL || width == NULL || height == NULL)
        return NULL;
    fseek(input, 0, SEEK_END);
    size_t filesize = ftell(input);
    rewind(input);
    char* maze = malloc(sizeof(*maze) * (filesize + 1));
    fgets(maze, filesize + 1, input);
    size_t n = strnlen(maze, filesize) - 1;
    size_t m = 0; //n for width, m for height
    do
    {
        m++;
    } while (fgets(maze + n * m, filesize + 1, input) != NULL);
    *(maze + n * m) = '\0';
    *width = n;
    *height = m;
    return maze;
}

int isObstacle(char c, const char* obstacle)
{
    int i = 0;
    while (obstacle[i] != '\0' && obstacle[i] != c)
        i++;
    if (obstacle[i] == '\0')
        return 0;
    return 1;
}

void neighbourHandler(const char* maze, int* distance, int neighbour, const char* obstacles, Queue* queue, int len)
{
    if (distance[neighbour] == 0 && !isObstacle(maze[neighbour], obstacles))
    {
        distance[neighbour] = len;
        queuePush(queue, neighbour);
    }
}

int findPosition(char* maze, char c)
{
    int i = 0;
    while(maze[i] != '\0')
    {
        if (maze[i] == c)
            return i;
        i++;
    }
    return -1;
}

// last character int return value is marked with -1
int* findDistances(const char* maze, size_t width, size_t height, int start, int end, const char* obstacles)
{
    if (maze == NULL || obstacles == NULL)
        return NULL;

    int totalSize = width * height;
    int* distance = calloc(totalSize, sizeof(*distance));
    if (distance == NULL)
        return NULL;

    Queue* queue = queueConstruct(totalSize);
    if (queue == NULL)
    {
        free(distance);
        return NULL;
    }
    int len = 1;
    int neighbour, current, column;
    queuePush(queue, start);
    distance[start] = len;
    while(!queueEmpty(queue))
    {
        current = queuePop(queue);
        //
        if (current == end)
        {
            queueDestruct(queue);
            return distance;
        }
        //
        len = distance[current] + 1;
        //Hope for a suggestion of a more elegant solution of copy-paste than neighbourHandler
        //up
        neighbour = current - width;
        if (neighbour >= 0)
            neighbourHandler(maze, distance, neighbour, obstacles, queue, len);
        //down
        neighbour = current + width;
        if (neighbour < totalSize)
            neighbourHandler(maze, distance, neighbour, obstacles, queue, len);
        column = current % width;
        //left
        if (column != 0)
            neighbourHandler(maze, distance, current - 1, obstacles, queue, len);  
        //right
        if (column != width - 1)
            neighbourHandler(maze, distance, current + 1, obstacles, queue, len);
    }
    queueDestruct(queue);
    return (int*) distance;
}


int findPath(char* maze, int* distance, size_t width, size_t height, int start, int end, FILE* file)
{
    if (distance[end] == 0)
    {
        return 0;
    }
    int neighbour, current, column;
    int totalSize = width * height;
    current = end;

    while(current != start)
    {
        //up
        neighbour = current - width;
        while (neighbour >= 0 && distance[current] - distance[neighbour] == 1 && (current = neighbour) != start)
            maze[current] = '+';

        //down
        neighbour = current + width;
        while (neighbour < totalSize && distance[current] - distance[neighbour] == 1 && (current = neighbour) != start)
            maze[current] = '+';

        column = current % width;

        //left
        while (column != 0 && distance[current] - distance[current - 1] == 1 && (current -= 1) != start)
            maze[current] = '+';

        //right
        while (column != width - 1 && distance[current] - distance[current + 1] == 1 && (current += 1) != start)
                maze[current] = '+';

    }
    
    return distance[end] - distance[start];
}

int putTile(char* maze, int pos, int start, int end, char* obstacles)
{
    if (pos == start)
    {
        printf("\x1b[35m%c", maze[pos]);    // violet
        return 0;
    }

    if (pos == end)
    {
        printf("\x1b[33m%c", maze[pos]);    // yellow
        return 0;
    }

    if (maze[pos] == '+')
    {
        printf("\x1b[32m%c", maze[pos]);    // green
        return 0;
    }

    if (isObstacle(maze[pos], obstacles))
    {
        printf("\x1b[31m%c", maze[pos]);    //red
        return 0;
    }

    printf("\x1b[37m%c", maze[pos]);    //grey
    return 0;
}

int printPath(char* maze, int pathLength, size_t width, size_t height, int start, int end, FILE* file, char* obstacles)
{
    if (pathLength == 0)
    {
        fprintf(file, "There is no way out of this\n");
        return 0;
    }

    if (file == stdout)
    {
        int i, j;
        for(i = 0; i < height; i++)
        {
            for(j = 0; j < width - 1; j++)
            {
                putTile(maze, i * width + j, start, end, obstacles);
                putchar(' ');
            }
        putTile(maze, i * width + width - 1, start, end, obstacles);
        putchar('\n');
        }

        printf("\x1b[0m\nTotal path length is %d\n", pathLength);
        return 0;
    }

    int i, j;
    for(i = 0; i < height; i++)
    {
        for(j = 0; j < width - 1; j++)
        {
            fputc(maze[i * width + j], file);
            fputc(' ', file);
        }
        fputc(maze[i * width + width - 1], file);
        fputc('\n', file);
    }

    fprintf(file, "\nTotal path length is %d\n", pathLength);
    return 0;
}

int main(int argc, char** argv)
{
    // input and output in file
    // cmd line format: ./a.out in out
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }

    FILE* input = fopen(argv[1], "r");
    if (input == NULL)
    {
        fprintf(stderr, "Input file has't been opened\n");
        return 1;
    }

    FILE* output = NULL;
    char stdoutText[] = "stdout";
    if (strcmp(stdoutText, argv[2]) == 0)
        output = stdout;
    else
    {
        output = fopen(argv[2], "w");
        if (output == NULL)
        {
            fclose(output);
            fprintf(stderr, "Output file has't been opened\n");
            return 1;
        }
    }

    size_t width, height;
    char* maze = getMaze(input, &width, &height);
    fclose(input);
    char obstacles[] = {'#'};

    int start = findPosition(maze, 'H');
    int end = findPosition(maze, '$');
    int* distance = findDistances(maze, width, height, start, end, obstacles);
    int pathLength = findPath(maze, distance, width, height, start, end, output);
    printPath(maze, pathLength, width, height, start, end, output, obstacles);
    fclose(output);
    free(maze);
    free(distance);
    return 0;
}
