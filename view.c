// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "view.h"

int main(int argc, char const *argv[]){

    size_t memSize;
    
    if( argc == 1 ){
        char buffer[MAX_MEM_CHARS] = {0};
        if( read(STDIN, buffer, MAX_MEM_CHARS) == ERROR ) {
            handle_error("Read failed");
        }
        memSize = atoi(buffer);
    } else if( argc == 2 ) {
        memSize = atoi(argv[1]);
    } else {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(EXIT_FAILURE);
    }

    shmData shmView;

    shmView.shmPtr = openShm(memSize);
    shmView.shmIndex = shmView.shmPtr;

    if( ( shmView.sem = sem_open(SEM_NAME, O_CREAT, S_RWALL, 0) ) ==  SEM_FAILED ) {
        handle_error("sem_open failed");
    }

    size_t tasks = memSize / MAX_BUFF;
    
    printTasks(tasks, &shmView);

    closeShm(shmView.shmPtr, memSize);

    closeSem(shmView.sem);

    return 0;
}

void printTasks(int tasks, shmData * shmView) {

    size_t printedTasks = 0;
    
    while( printedTasks < tasks ){
        if( sem_wait(shmView->sem) == ERROR ){
            handle_error("Sem_wait failed");
        }

        int printed = printf("%s\n", shmView->shmIndex); 
        shmView->shmIndex += printed; //It already includes + 1 from \n
        
        printedTasks++; 
    }
}

char* openShm(size_t memSize){
    
    int shmFd;
    if( ( shmFd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_RWALL) ) == ERROR) {
        handle_error("Shm_open failed");
    }

    void * ptr;
    if( ( ptr = mmap(0, memSize, PROT_WRITE, MAP_SHARED, shmFd, 0) ) == MAP_FAILED) {
        handle_error("Mmap failed");
    }

    if( close(shmFd) == ERROR ) {
        handle_error("Close failed");
    }

    return (char*) ptr;
}

void closeShm(char * ptr, size_t memSize){

    if ( munmap(ptr, memSize) == ERROR ){
        handle_error("munmap failed");
    }
}

void closeSem(sem_t * sem){

    if( sem_close(sem) == ERROR ){
        handle_error("Sem_close failed");
    }
}