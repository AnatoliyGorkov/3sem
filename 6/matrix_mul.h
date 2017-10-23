#ifndef MATRIX_MUL_USAGE

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

typedef float mvalue_t;

typedef struct commonParams_t_{
    int n;
    mvalue_t* matrixA;
    mvalue_t* matrixB;
    mvalue_t* result;
} commonParams_t;
typedef struct params_t_ {
    int start;  //includes start
    int end;    //does not include end itself
    commonParams_t* commonParams;
} params_t;

int transponate(mvalue_t* matrix, size_t n)
{
    int i, j;
    mvalue_t swap;
    for (i = 1; i < n; i++)
        for (j = 0; j < i; j++)
        {
            /*
                лучще даже небольшие куски кода, которые выполняют нечто законченное, выносить в отдельную ф-ю
                ... void swap(mvalue_t* lhs, mvalue_t* rhs);
            */
            swap = *(matrix + i * n + j);
            *(matrix + i * n + j) = *(matrix + j * n + i);
            *(matrix + j * n + i) = swap;
        }
    return 0;
}

void* threadMultiply(void* args)  //second matrix is taken transponated
{
    int i, j, k;
    params_t* params = (params_t*) args;
    mvalue_t* a = (params -> commonParams) -> matrixA;
    mvalue_t* b = (params -> commonParams) -> matrixB;
    mvalue_t* c = (params -> commonParams) -> result;
    int n = (params -> commonParams) -> n;
    for (i = params -> start; i < params -> end; i++)
        for (j = 0; j < n; j++)
            for (k = 0; k < n; k++)
                *(c + i * n + j) += (*(a + i * n + k)) * (*(b + j * n + k)); //c[i][j] += a[i][k] * b[j][k]
    /*
    https://stackoverflow.com/questions/20467117/for-matrix-operation-why-is-ikj-faster-than-ijk
    можно халявно заставить код работать побыстрее, поменяв порядок обращения к памяти
    */
    pthread_exit(NULL);
    /*
    А зачем здесь pthread_exit явно писать? Нить же итак завершится, дойдя до закрывающейся скобки.
    */
}

mvalue_t* matrixMul(mvalue_t* matrixA, mvalue_t* matrixB, size_t n, size_t numberOfThreads)
{
    if (matrixA == NULL || matrixB == NULL)
        return NULL;
    mvalue_t* matrixC = (mvalue_t*) calloc(n * n, sizeof(*matrixC));
    if (matrixC == NULL)
        return NULL;
    
    int s = n % numberOfThreads; // s is the number of processes where no. of lines = linesPerThread + 1
    pthread_t* threads = (pthread_t*) malloc(numberOfThreads * sizeof(*threads));
    int linesPerThread = n / numberOfThreads;
    int i;
    transponate(matrixB, n);  //to make cache usage more effective
    params_t* params = (params_t*) malloc(numberOfThreads * sizeof(*params));
    commonParams_t commonParams = {n, matrixA, matrixB, matrixC};

    // for s "exetended" threads
    linesPerThread++;
    int threadNumber;
    int lineCounter = 0;
    for(threadNumber = 0; threadNumber < s; threadNumber++)
    {
        params[threadNumber].start = lineCounter;
        params[threadNumber].end = lineCounter + linesPerThread;
        params[threadNumber].commonParams = &commonParams;
        pthread_create(threads + threadNumber, NULL, threadMultiply, params + threadNumber);
        lineCounter += linesPerThread;
    }
    // for (numberOfThreads - s) "normal" threads
    linesPerThread--;
    for(threadNumber = s; threadNumber < numberOfThreads && linesPerThread > 0; threadNumber++)
    {
        params[threadNumber].start = lineCounter;
        params[threadNumber].end = lineCounter + linesPerThread;
        params[threadNumber].commonParams = &commonParams;
        pthread_create(threads + threadNumber, NULL, threadMultiply, params + threadNumber);
        lineCounter += linesPerThread;
    }
    for(threadNumber = 0; threadNumber < numberOfThreads; threadNumber++)
        pthread_join(threads[threadNumber], NULL);

    transponate(matrixB, n);  //to return B in initial state
    free(threads);
    free(params);
    return matrixC;
}

#define MATRIX_MUL_USAGE 
#endif
