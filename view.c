// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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

#define STDIN 0
#define STDOUT 1
#define MAX_MEM_CHARS 21

static char * shmPtr;
static char * shmIndex;

static sem_t * sem;

char* openShm(size_t memSize);
void closeShm(char* ptr, size_t memSize);
void closeSem();
void printTasks(int tasks);

int main(int argc, char const *argv[]){

    size_t memSize;
    char buffer[MAX_MEM_CHARS] = {0};

    if( argc == 1 ){
        read(STDIN, buffer, MAX_MEM_CHARS);
        memSize = atoi(buffer);
    } else if( argc == 2) {
        memSize = atoi(argv[1]);
    } else {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(ARGUMENTS_ERR);
    }
        
    //Memoria compartida
    shmPtr = openShm(memSize);
    shmIndex = shmPtr;
    
    //Semaforo
    if( (sem = sem_open(SEM_NAME, O_CREAT, S_RWALL, 0)) ==  SEM_FAILED ) {
        handle_error("sem_open failed");
    }

    size_t tasks = memSize / MAX_BUFF;
    
    printTasks(tasks);

    //Finalizacion del programa
    closeShm(shmPtr, memSize);

    closeSem();

    return 0;
}

void printTasks(int tasks) {
    size_t printedTasks = 0;
    
    while( printedTasks < tasks ){
        if( sem_wait(sem) == ERROR){
            handle_error("Sem_wait failed");
        }

        int printed = printf("%s\n", shmIndex);

        if( printed < 0 ){
            handle_error("Printf failed");
        }

        shmIndex += printed; //ya incluye +1 del \n
        printedTasks++; 
    }
}

char* openShm(size_t memSize){
    int shmFd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_RWALL);

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
    if (munmap(ptr, memSize) == ERROR){
        handle_error("munmap failed");
    }
}

void closeSem(){

    if( sem_close(sem) == ERROR ){
        handle_error("Sem_close failed");
    }
}