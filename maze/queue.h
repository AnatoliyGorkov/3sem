#ifndef QUEUE_USAGE
//typedef int qvalue_t;
#include <stdlib.h>

typedef struct Queue_ {
    qvalue_t* data;
    size_t size;
    size_t fill;    //number of elements in the queue
    size_t head;
    size_t tail;
} Queue;

Queue* queueConstruct(size_t size)
{
    if (size == 0)
        return NULL;
    Queue* queue = (Queue*) malloc(sizeof(*queue));
    if (queue == NULL)
        return NULL;

    qvalue_t* data = (qvalue_t*) malloc(size * sizeof(*data));
    if (data == NULL)
    {
        free(queue);
        return NULL;
    }

    queue -> data = data;
    queue -> head = 0;
    queue -> tail = 0;
    queue -> fill = 0;
    queue -> size = size;
    return queue;
}

int queueDestruct(Queue* queue)
{
    if (queue == NULL)
        return 1;
    if (queue -> data == NULL)
    {
        free(queue);
        return 0;
    }
    free(queue -> data);
    free(queue);
    return 0;
}

int queuePush(Queue* queue, qvalue_t value)
{
    if (queue == NULL || queue -> data == NULL)
        return -1;
    if (queue -> fill == queue -> size)
        return 1;
    *(queue -> data + queue -> tail) = value;
    queue -> fill++;
    queue -> tail++;
    return 0;
}

qvalue_t queuePop(Queue* queue)
{
    if (queue == NULL || queue -> data == NULL)
        return 0;
    if (queue -> fill == 0)
        return 0;
    queue -> fill--;
    return *(queue -> data + queue -> head++);
}

int queueEmpty(Queue* queue)
{
    if (queue == NULL)
        return -1;
    if (queue -> fill == 0)
        return 1;
    return 0;
}

#define QUEUE_USAGE 1
#endif
