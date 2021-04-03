// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#define _XOPEN_SOURCE 500

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

#include <semaphore.h>

#include "sharedDefs.h"



#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1
#define MAX_SLAVES 5
#define INITIAL_TASKS 2
#define TRUE 1
#define FALSE 0
#define max(x,y) ((x) > (y) ? (x) : (y))

typedef struct process {
    pid_t pid;
    size_t remainingTasks;
    int inFd; //desde el esclavo al master
    int outFd; //desde el master al esclavo
    int isWorking;
    //Por donde entra y por donde sale
} process;

static int assignedTasks = 0, totalTasks;
static size_t finishedTasks = 0;
static size_t totalSlaves;

static char * shmPtr;
static char * shmIndex;

static sem_t * sem;

static FILE * result;

void createSlaves(process slaves[], char ** tasks);
int parseToken(char * buffer, char delimiter);
int fillSet(fd_set * fdSet, process slaves[]);
void handleBuffer(process * slave, fd_set *fdSet, char ** tasks);
void processTasks(process slaves[], char ** tasks);

char* openShm(size_t memSize);
void closeShm(char* ptr, size_t memSize);

void closeSem();


int main(int argc, char const *argv[]){

    if(argc < 2) {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(ARGUMENTS_ERR);
    }

    totalTasks = argc - 1;

    char ** tasks = (char**) argv + 1;
    totalSlaves = (totalTasks < MAX_SLAVES) ? totalTasks : MAX_SLAVES;

    size_t memSize = totalTasks * MAX_BUFF;

    //Memoria compartida
    shmPtr = openShm(memSize);
    shmIndex = shmPtr;

    //Semaforo
    //Semantica del semaforo: ¿Cuantos resultados hay para leer?
    if( (sem = sem_open(SEM_NAME, O_CREAT, S_RWALL, 0)) ==  SEM_FAILED ) {
        handle_error("sem_open failed");
    }

    //wait for vista
    if( setvbuf(stdout, NULL, _IONBF, 0) != 0 ){
        handle_error("Setvbuf failed");
    } 
    printf("%ld", memSize);
    sleep(2);

    //Abrimos el archivo FILE * result
    if((result = fopen("result.txt", "w")) == NULL) {
        handle_error("Fopen failed");
    }
    
    //Creación de esclavos
    process slaves[totalSlaves];

    createSlaves(slaves, tasks);
    
    //Proceso de tareas
    processTasks(slaves, tasks);

    //Finalización del programa 
    closeShm(shmPtr, memSize); //Revisar si aqui esta bien puesto

    closeSem();

    if(fclose(result) == EOF) {
        handle_error("Fclose failed");
    }
    //printf("\n");
    return 0;
}

void processTasks(process slaves[], char ** tasks){
    fd_set fdSet;
    int maxFd;

    while( finishedTasks < totalTasks ){
        FD_ZERO(&fdSet);
        maxFd = fillSet(&fdSet, slaves);

        int toRead = select(maxFd + 1, &fdSet, NULL, NULL, NULL);
        if(toRead == -1){
            handle_error("Select failed");
        }

        for(int i = 0; toRead > 0 && i < totalSlaves; i++){
            handleBuffer(&slaves[i], &fdSet, tasks);
        }
    }
}

void handleBuffer(process * slave, fd_set *fdSet, char ** tasks){
    int fd = slave->inFd;
    char readBuffer[MAX_BUFF] = {0};
    
    if( FD_ISSET(fd, fdSet) ){
        size_t amount = 0;

        if( ( amount = read(fd, readBuffer, MAX_BUFF) ) == ERROR ){
            handle_error("Read failed");
        }

        if( amount == 0 ){
            if( close(fd) == ERROR ) {
                handle_error("Close failed");
            }
            slave->isWorking = FALSE;
        } else {
            char * token;
            token = strtok(readBuffer, "\t");
            while( token != NULL ) {
                finishedTasks++;
                slave->remainingTasks--;
                
                if( fprintf(result, "%s\n", token) < 0 ){
                    handle_error("Fprintf failed");
                }

                int printed = sprintf(shmIndex, "%s", token);
                if( printed < 0 ){
                    handle_error("sprintf failed");
                }
                shmIndex += printed + 1; 
                
                if( sem_post(sem) == ERROR){
                    handle_error("Sem_post failed");
                }

                token = strtok(NULL, "\t");
            }

            if( assignedTasks < totalTasks && slave->remainingTasks == 0 ){
                char * writeBuffer = tasks[assignedTasks++];
                
                if( write(slave->outFd, writeBuffer, strlen(writeBuffer) + 1 ) == ERROR ) {
                    handle_error("Write failed");
                }

                slave->remainingTasks++;

            } else if( assignedTasks == totalTasks ) {
                if( close(slave->outFd) == ERROR ) {
                    handle_error("Close failed");
                }
            }
        }
    }
}

int fillSet(fd_set * fdSet, process slaves[]) {
    FD_ZERO(fdSet);
    int fd;
    int maxFd = -1;

    for(int i = 0; i < totalSlaves; i++) {
        if( slaves[i].isWorking ){
            fd = slaves[i].inFd;
            FD_SET(fd, fdSet);
            maxFd = max(maxFd, fd);              
        }
    }
    return maxFd;
}

void createSlaves(process slaves[], char ** tasks){    
    //Si totalTasks >= INITIAL * totalSlaves -> Mandar INITIAL files al crearlos
    int paths[2], answers[2];
    int pid;

    int initialTasks = (totalTasks >= INITIAL_TASKS * totalSlaves) ? INITIAL_TASKS : 1;
    char *execParams[initialTasks + 2];
    execParams[0] = "./slave";
    execParams[initialTasks + 1] = NULL;


    for (int i = 0; i < totalSlaves; i++)
    {
        
        if( pipe(paths) == ERROR || pipe(answers) == ERROR ){
            handle_error("Pipe failed");
        }
        
        if( ( pid = fork() ) == 0 ){

            for (int j = 1; j <= initialTasks; j++){
                execParams[j] = tasks[assignedTasks++];
            }
            
            if( dup2(paths[READ], STDIN) == ERROR || dup2(answers[WRITE], STDOUT) == ERROR ){
                handle_error("Dup2 failed");
            }

            if( close(paths[READ]) == ERROR || close(paths[WRITE]) == ERROR 
                    || close(answers[READ]) == ERROR || close(answers[WRITE]) == ERROR ){
                handle_error("Close failed");
            }

            execv(execParams[0], execParams);
            handle_error("Exec failed");

        }else if( pid == ERROR ){
            handle_error("Fork failed");
        }

        slaves[i].pid = pid;
        slaves[i].remainingTasks = initialTasks;
        slaves[i].inFd = answers[READ];
        slaves[i].outFd = paths[WRITE];
        slaves[i].isWorking = TRUE;
        
        assignedTasks += initialTasks;

        if( close(paths[READ]) == ERROR || close(answers[WRITE]) == ERROR ) {
            handle_error("Close failed");
        }
    }
}

char* openShm(size_t memSize){

    int shmFd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_RWALL);
    
    if(shmFd == ERROR){
        handle_error("Shm_open failed");
    }

    if( ftruncate(shmFd, memSize) == ERROR ){
        handle_error("Ftruncate failed");
    }

    void * ptr = mmap(0, memSize, PROT_WRITE, MAP_SHARED, shmFd, 0);

    if(close(shmFd) == ERROR) {
        handle_error("Close failed");
    }
     
    return (char *) ptr;
}

void closeShm(char* ptr, size_t memSize){

    if (munmap(ptr, memSize)== ERROR){
        handle_error("munmap failed");
    }
    
    if (shm_unlink(SHM_PATH) == ERROR){
        handle_error("shm_unlink failed");
    }
}

void closeSem(){

    if( sem_close(sem) == ERROR ){
        handle_error("Sem_close failed");
    }

    if( sem_unlink(SEM_NAME) == ERROR ){
        handle_error("Sem_unlink failed");
    }

}