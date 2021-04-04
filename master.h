#ifndef MASTER_H
#define MASTER_H

#define _XOPEN_SOURCE 500

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <semaphore.h>

#include "sharedDefs.h"
#include "errorCons.h"

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
    int inFd;
    int outFd;
    int isWorking;
} process;

typedef struct tasksData{
    int assignedTasks;
    int totalTasks;
    size_t finishedTasks;
    size_t totalSlaves;
} tasksData;

void createSlaves(process slaves[], char ** tasks, tasksData * tasksMaster);
int parseToken(char * buffer, char delimiter);
int fillSet(fd_set * fdSet, process slaves[], size_t totalSlaves);
void handleBuffer(process * slave, fd_set * fdSet, char ** tasks, tasksData * tasksMaster, shmData * shmMaster, FILE * result);
void processTasks(process slaves[], char ** tasks, tasksData *tasksMaster, shmData * shmMaster, FILE * result);
char * openShm(size_t memSize);
void closeShm(char * ptr, size_t memSize);
void closeSem(sem_t * sem);

#endif