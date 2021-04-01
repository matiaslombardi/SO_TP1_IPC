#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include "errorCons.h"

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

void createSlaves(process slaves[], char ** tasks, int totalTasks, int* tasksAssigned, int totalSlaves);

int main(int argc, char const *argv[])
{
    if(argc < 2) {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(ARGUMENTS_ERR);
    }

    int tasksAssigned = 0;
    int totalTasks = argc - 1;
    size_t finishedTasks = 0;

    char ** tasks = (char**) argv + 1;
    size_t totalSlaves = (totalTasks < MAX_SLAVES) ? totalTasks : MAX_SLAVES;

    process slaves[totalSlaves];

    createSlaves(slaves, tasks, totalTasks, &tasksAssigned, totalSlaves);

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

        for(int i = 0; toRead > 0 && i < totalSlaves ; i++){
            fd = slaves[i].inFd;
            char buffer[MAX_BUFF] = {0};
            if( FD_ISSET(fd, &fdSet) ){
                size_t amount = 0;
                if( amount = read(fd, buffer, MAX_BUFF) == -1 ){
                    //hermano tanto vas a fallar???
                }
                if(amount == 0){

                }
            }
        }
    }
    return 0;
/*
    int count = 0;
    char newAnswer[MAX_BUFF] = {0}; 
    //write(paths[WRITE], argv[1], strlen(argv[1]));
    count = read(answers[READ], newAnswer, MAX_BUFF);
    printf("%s\n", newAnswer);

    newAnswer[count] = 0;
    write(paths[WRITE], argv[2], strlen(argv[2]));
    read(answers[READ], newAnswer, MAX_BUFF);
    printf("%s\n", newAnswer);

    newAnswer[count] = 0;
    write(paths[WRITE], argv[3], strlen(argv[3]));
    read(answers[READ], newAnswer, MAX_BUFF);
    printf("%s\n", newAnswer);
*/
}

void createSlaves(process slaves[], char ** tasks, int totalTasks, int* tasksAssigned, int totalSlaves){    
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
                execParams[j] = tasks[(*tasksAssigned)++];
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
        
        *tasksAssigned += initialTasks;

        if( close(paths[READ]) == ERROR || close(answers[WRITE]) == ERROR ){
                //mas manejo de errores... BASTA
        }
    }
}
