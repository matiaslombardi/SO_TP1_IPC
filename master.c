#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include "errorCons.h"
#include <sys/select.h>

#define MAX_BUFF 1024
#define READ 0
#define WRITE 1
#define STDIN 0
#define STDOUT 1
#define MAX_SLAVES 5
#define INITIAL_TASKS 2
#define TRUE 1
#define FALSE 0

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

void createSlaves(process slaves[], char ** tasks);
void processTasks(process slaves[], char ** tasks);

int main(int argc, char const *argv[])
{
    argc = 1;
    if(argc < 2) {
        handle_error("Incorrect amount of arguments");
        //fprintf(stderr,"%s\n","Incorrect amount of arguments");
        //exit(ARGUMENTS_ERR);
    }

    totalTasks = argc - 1;

    char ** tasks = (char**) argv + 1;
    totalSlaves = (totalTasks < MAX_SLAVES) ? totalTasks : MAX_SLAVES;

    process slaves[totalSlaves];

    createSlaves(slaves, tasks);

    processTasks(slaves, tasks);

    return 0;
}

void processTasks(process slaves[], char ** tasks){

    fd_set fdSet;
    int maxFd = -1;

    while( finishedTasks < totalTasks ){
        FD_ZERO(&fdSet);
        int fd;
        for(int i = 0; i < totalSlaves; i++) {
            if( slaves[i].isWorking ){
                fd = slaves[i].inFd;
                FD_SET(fd, &fdSet);

                if( fd > maxFd )
                    maxFd = fd;               
            }
        }

        int toRead = select(maxFd + 1, &fdSet, NULL, NULL, NULL);
        if(toRead == -1){
            //basta hermano de errores
        }

        for(int i = 0; toRead > 0 && i < totalSlaves; i++){
            fd = slaves[i].inFd;
            char readBuffer[MAX_BUFF] = {0};
            if( FD_ISSET(fd, &fdSet) ){
                size_t amount = 0;

                if( ( amount = read(fd, readBuffer, MAX_BUFF) ) == ERROR ){
                    //hermano tanto vas a fallar???
                }

                if(amount == 0){
                    slaves[i].isWorking = FALSE;
                }else{
                    //PARSEAR TAREAS
                    char * token;
                    token = strtok(readBuffer, "\t");
                    while( token != NULL ) {
                        finishedTasks++;
                        slaves[i].remainingTasks--;
                        //printf("TOKEN: %s",token);
                        
                        printf("Tarea %ld\n", finishedTasks);
                        printf("finished tasks: %ld\n", finishedTasks);
                        printf("total tasks: %d\n", totalTasks);
                        printf("assigned tasks: %d\n", assignedTasks);
                        printf("token: %s\n", token);
                        printf("-------------------\n");
                        token = strtok(NULL, "\t");
                        //break;
                        //hay que mandar a la shm
                    }

                    if( assignedTasks < totalTasks && slaves[i].remainingTasks == 0){
                        char * writeBuffer = tasks[assignedTasks++];
                        if( write(slaves[i].outFd, writeBuffer, strlen(writeBuffer)) == ERROR ) {
                            //Salida error :) o tal vez :(
                        }
                        slaves[i].remainingTasks++;
                    } else if( assignedTasks == totalTasks ) {
                        if( close(slaves[i].outFd) == ERROR ) {
                            //Salida error :(
                        }
                    }

                }
            }
        }
    }
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
            //errores
            return;
        }
        
        if( ( pid = fork() ) == 0 ){

            for (int j = 1; j <= initialTasks; j++){
                execParams[j] = tasks[assignedTasks++];
            }
            
            if( dup2(paths[READ], STDIN) == ERROR || dup2(answers[WRITE], STDOUT) == ERROR ){
                //mas manejo de errores... BASTA
            }

            if( close(paths[READ]) == ERROR || close(paths[WRITE]) == ERROR 
                    || close(answers[READ]) == ERROR || close(answers[WRITE]) == ERROR ){
                //mas manejo de errores... BASTA
            }

            execv(execParams[0], execParams);
            //si esta aca falló, manejoooooo de Rerrrrrrorres (te falto una r)

        }else if( pid == ERROR ){
            //otra vez...
        }

        slaves[i].pid = pid;
        slaves[i].remainingTasks = initialTasks;
        slaves[i].inFd = answers[READ];
        slaves[i].outFd = paths[WRITE];
        slaves[i].isWorking = TRUE;
        
        assignedTasks += initialTasks;

        if( close(paths[READ]) == ERROR || close(answers[WRITE]) == ERROR ){
                //mas manejo de errores... BASTA
        }
    }
}
