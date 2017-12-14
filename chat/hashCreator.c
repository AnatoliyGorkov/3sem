#include "common.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Error\n");
        return 1;
    }
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256((unsigned char*) argv[1], strlen(argv[1]), hash);
    int fd = open(PASWDFILE, O_CREAT | O_WRONLY, 0666);
    write(fd, hash, SHA256_DIGEST_LENGTH);
    return 0;
}
