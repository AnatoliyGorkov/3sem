#include "common.h"

int main(int argc, char** argv)
{
    // input format: ./wiper wiperSpeed.txt TABLE_LIMIT
    if (argc != 3)
    {
        fprintf(stderr, "Incorrect input parameters\n");
        return 1;
    }

    int tableSize = atoi(argv[2]);

    int* speed = getSpeed(argv[1]);

    key_t sem1key = ftok(KEYFILE, SEMNUM1);
    key_t sem2key = ftok(KEYFILE, SEMNUM2);
    key_t sem3key = ftok(KEYFILE, SEMNUM3);
    key_t shmkey = ftok(KEYFILE, SHMNUM);

    int sem1 = semget(sem1key, 1, IPC_CREAT | 0666);
    int sem2 = semget(sem2key, 1, IPC_CREAT | 0666);    //sem2 is for free space
    int sem3 = semget(sem3key, 1, IPC_CREAT | 0666);    //sem3 if for taken space
    int shmid = shmget(shmkey, tableSize * sizeof(int), IPC_CREAT | 0666);
    // printf("%d\n", shmid);
    int* table = shmat(shmid, NULL, 0);
    int dishType;
    int tablePosition = 0;
    printTime();
    printf(" Came to work. Waiting for washer\n");
    sOp(sem1, -1);
    while(1)
    {
        printTime();
        printf(" Getting the dish\n");
        sOp(sem2, 1);
        sOp(sem3, -1);

        sOp(sem1, -1);
        dishType = table[tablePosition];
        tablePosition++;
        if (tablePosition >= tableSize)
            tablePosition -= tableSize;
        sOp(sem1, 1);

        if (dishType == -1)
            break;
        printTime();
        printf(" Wiping %d\n", dishType);
        sleep(speed[dishType]);
    }

   
    printTime();
    printf(" Finished\n");
    shmdt(table);
    free(speed);
    sOp(sem1, -2);

    semctl(sem1, IPC_RMID, 0);
    semctl(sem2, IPC_RMID, 0);
    semctl(sem3, IPC_RMID, 0);
    shmctl(shmid, IPC_RMID, 0);

    return 0;
}