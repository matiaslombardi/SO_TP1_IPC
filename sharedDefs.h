#ifndef SHARED_H
#define SHARED_H

#define SHM_PATH "/shared"
#define MAX_BUFF 4096
#define SEM_NAME "/sem"
#define S_RWALL (S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH) 

typedef struct shmData{
    char * shmPtr;
    char * shmIndex;
    sem_t * sem;
} shmData;

#endif