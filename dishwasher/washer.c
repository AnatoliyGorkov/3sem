#include "common.h"

int main(int argc, char** argv)
{
    // input format: ./washer washerSpeed.txt dishes.txt TABLE_LIMIT
    //n dish types are indicated with integer numbers [0, n - 1]
    if (argc != 4)
    {
        fprintf(stderr, "Incorrect input parameters\n");
        return 1;
    }

    int tableSize = atoi(argv[3]);
    FILE* dishesFile = fopen(argv[2], "r");
    if (dishesFile == NULL)
    {
        fprintf(stderr, "File with dished cannot be opened\n");
        return 1;
    }

    int* speed = getSpeed(argv[1]);

    key_t sem1key = ftok(KEYFILE, SEMNUM1);
    key_t sem2key = ftok(KEYFILE, SEMNUM2);
    key_t sem3key = ftok(KEYFILE, SEMNUM3);
    key_t shmkey = ftok(KEYFILE, SHMNUM);

    int sem1 = semget(sem1key, 1, IPC_CREAT | 0666);
    int sem2 = semget(sem2key, 1, IPC_CREAT | 0666);
    int sem3 = semget(sem3key, 1, IPC_CREAT | 0666);
    int shmid = shmget(shmkey, tableSize * sizeof(int), IPC_CREAT | 0666);
    int* table = shmat(shmid, NULL, 0);

    int dishType, dishNumber, i;
    int tablePosition = 0;
    sOp(sem1, 2);
    sOp(sem2, tableSize);

    while(fscanf(dishesFile, "%d%d", &dishType, &dishNumber) == 2)
    {
        for(i = 0; i < dishNumber; i++)
        {
            printTime();
            printf(" Washing %d %d\n", dishType, speed[dishType]);
            sleep(speed[dishType]);
            printTime();
            printf(" Putting the plate on the table\n");
            sOp(sem2, -1);
            sOp(sem3, 1);

            sOp(sem1, -1);
            table[tablePosition] = dishType;
            tablePosition++;
            if (tablePosition >= tableSize)
                tablePosition -= tableSize;
            sOp(sem1, 1);

        }
    }


    printTime();
    printf(" Finished\n");

    sOp(sem2, -1);
    sOp(sem3, 1);
    
    sOp(sem1, -1);
    table[tablePosition] = -1;
    sOp(sem1, 1);


    shmdt(table);
    fclose(dishesFile);
    free(speed);
    sOp(sem1, 2);
    printTime();
    printf(" Left work\n");
    return 0;
}