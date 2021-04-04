#ifndef VIEW_H
#define VIEW_H

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

#define STDIN 0
#define STDOUT 1
#define MAX_MEM_CHARS 21

char* openShm(size_t memSize);
void closeShm(char * ptr, size_t memSize);
void closeSem(sem_t * sem);
void printTasks(int tasks, shmData* shmView);

#endif