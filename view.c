#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include "errorCons.h"
#include <sys/select.h>
#include <errno.h>


#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

#include <sys/mman.h>

#include "sharedDefs.h"

#include <semaphore.h>


char* openShm(size_t memSize);
void closeShm(char* ptr, size_t memSize);

int main(int argc, char const *argv[]){
    size_t memSize = atoi(argv[1]);
    char* ptr = openShm(memSize);
    closeShm(ptr,memSize);
    return 0;
}

char* openShm(size_t memSize){
    int shmFd = shm_open(SHM_PATH, O_CREAT | O_RDWR, 0666);

    if(shmFd == ERROR){
        handle_error("Shm_open failed");
    }

    void * ptr = mmap(0, memSize, PROT_WRITE, MAP_SHARED, shmFd, 0);

    if(close(shmFd) == ERROR) {
        handle_error("Close failed");
    }

    return (char*) ptr;
}

void closeShm(char* ptr, size_t memSize){
    int retVal = munmap(ptr, memSize);
    if (retVal == ERROR){
        handle_error("munmap failed");
    }
}