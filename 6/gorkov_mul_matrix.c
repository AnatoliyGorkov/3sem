#include <stdlib.h>
#include <stdio.h>
#include "matrix_mul.h" //mvalue_t if defined in header file

mvalue_t* generateMatrix(size_t n) // generates matrix with floats in range [0,1]
{
    mvalue_t* matrix = malloc(n * n * sizeof(*matrix));
    if (matrix == NULL)
        return NULL;
    int i;
    for (i = 0; i < n * n; i++)
        matrix[i] = ((float) rand()) / RAND_MAX;
    return matrix;
}


int main(int argc, char** argv)
{

    if (argc != 3)
    {
        fprintf(stderr, "Incorrect command line parameters\n");
        return 1;
    }

    int numberOfThreads = atoi(argv[1]);
    int n = atoi(argv[2]);  //n is the size of the matrix

    mvalue_t* matrixA = generateMatrix(n); //must be freed after usage
    mvalue_t* matrixB = generateMatrix(n); //must be freed after usage
    mvalue_t* result = matrixMul(matrixA, matrixB, n, numberOfThreads);

    /*
    // for the test of correct work (tested on integer)
    int i, j;
    printf("Matrix A:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
            printf("%d ",  *(matrixA + i * n + j));
        printf("\n");
    }
    printf("______________________________________\n");

    printf("Matrix B:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
            printf("%d ",  *(matrixB + i * n + j));
        printf("\n");
    }
    printf("______________________________________\n");

    printf("Result:\n");
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < n; j++)
            printf("%d ",  *(result + i * n + j));
        printf("\n");
    }
    printf("______________________________________\n");
    */

    free(matrixA);
    free(matrixB);
    free(result);
    
    return 0;
}