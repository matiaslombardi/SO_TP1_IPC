// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "master.h"

int main(int argc, char const *argv[]){
    
    if(argc < 2) {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(EXIT_FAILURE);
    }

    tasksData tasksMaster;
    tasksMaster.assignedTasks = 0;
    tasksMaster.finishedTasks = 0;
    tasksMaster.totalTasks = argc - 1;
    tasksMaster.totalSlaves = (tasksMaster.totalTasks < MAX_SLAVES) ? tasksMaster.totalTasks : MAX_SLAVES;
    
    char ** tasks = (char**) argv + 1;

    size_t memSize = tasksMaster.totalTasks * MAX_BUFF;
    shmData shmMaster;
    shmMaster.shmPtr = openShm(memSize);
    shmMaster.shmIndex = shmMaster.shmPtr;

    if( (shmMaster.sem = sem_open(SEM_NAME, O_CREAT, S_RWALL, 0)) ==  SEM_FAILED ) {
        handle_error("sem_open failed");
    }

    if( setvbuf(stdout, NULL, _IONBF, 0) ){
        handle_error("Setvbuf failed");
    } 
    printf("%lu", (unsigned long) memSize);
    sleep(2);

    FILE * result;
    if( (result = fopen("result.txt", "w")) == NULL ) {
        handle_error("Fopen failed");
    }

    process slaves[tasksMaster.totalSlaves];

    createSlaves(slaves, tasks, &tasksMaster);

    processTasks(slaves, tasks, &tasksMaster, &shmMaster, result);
 
    closeShm(shmMaster.shmPtr, memSize);

    closeSem(shmMaster.sem);

    if( fclose(result) == EOF ) {
        handle_error("Fclose failed");
    }

    terminateSlaves(slaves, tasksMaster.totalSlaves);

    return 0;
}

void processTasks(process slaves[], char ** tasks, tasksData * tasksMaster, shmData * shmMaster, FILE * result){
    
    fd_set fdSet;

    while( tasksMaster->finishedTasks < tasksMaster->totalTasks ){
        FD_ZERO(&fdSet);
        int maxFd = fillSet(&fdSet, slaves, tasksMaster->totalSlaves);

        int toRead;
        if( ( toRead = select(maxFd + 1, &fdSet, NULL, NULL, NULL) ) == -1){
            handle_error("Select failed");
        }

        for(int i = 0; toRead > 0 && i < tasksMaster->totalSlaves; i++){
            if( FD_ISSET(slaves[i].inFd, &fdSet ) ) {
                handleBuffer(&slaves[i], &fdSet, tasks, tasksMaster, shmMaster, result);
                toRead--;
            }
        }
    }
}

void handleBuffer(process * slave, fd_set *fdSet, char ** tasks, tasksData *tasksMaster, shmData *shmMaster, FILE * result){
    int fd = slave->inFd;
    char readBuffer[MAX_BUFF + 1] = {0};
    size_t amount = 0;

    if( ( amount = read(fd, readBuffer, MAX_BUFF) ) == ERROR ){
        handle_error("Read failed");
    }

    if( amount == 0 ){
        slave->isWorking = FALSE;
    } else {
        
        char * token = strtok(readBuffer, "\t");
        while( token != NULL ) {
            tasksMaster->finishedTasks++;
            slave->remainingTasks--;
            
            if( fprintf(result, "%s\n", token) < 0 ){
                handle_error("Fprintf failed");
            }

            int printed;
            if( (printed = sprintf(shmMaster->shmIndex, "%s", token)) < 0 ){
                handle_error("sprintf failed");
            }
            shmMaster->shmIndex += printed + 1; 
            
            if( sem_post(shmMaster->sem) == ERROR){
                handle_error("Sem_post failed");
            }

            token = strtok(NULL, "\t");
        }

        if(slave->remainingTasks == 0) {
            if(tasksMaster->assignedTasks < tasksMaster->totalTasks ){
                char * writeBuffer = tasks[tasksMaster->assignedTasks++];
            
                if( write(slave->outFd, writeBuffer, strlen(writeBuffer) + 1 ) == ERROR ) {
                    handle_error("Write failed");
                }
            
                slave->remainingTasks++;
            } else {
                if( close(slave->outFd) == ERROR ) {
                    handle_error("Close failed");
                }   
            }
        }
    }
}

int fillSet(fd_set * fdSet, process slaves[], size_t totalSlaves) {
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

void createSlaves(process slaves[], char ** tasks, tasksData * tasksMaster){    

    int paths[2], answers[2];

    int initialTasks = (tasksMaster->totalTasks >= INITIAL_TASKS * tasksMaster->totalSlaves) ? INITIAL_TASKS : 1;
    char *execParams[initialTasks + 2];
    execParams[0] = "./slave";
    execParams[initialTasks + 1] = NULL;

    for (int i = 0; i < tasksMaster->totalSlaves; i++)
    {
        if( pipe(paths) == ERROR || pipe(answers) == ERROR ){
            handle_error("Pipe failed");
        }
        int pid;
        if( ( pid = fork() ) == 0 ){

            for (int j = 1; j <= initialTasks; j++){
                execParams[j] = tasks[tasksMaster->assignedTasks++];
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
        
        tasksMaster->assignedTasks += initialTasks;

        if( close(paths[READ]) == ERROR || close(answers[WRITE]) == ERROR ) {
            handle_error("Close failed");
        }
    }
}

char* openShm(size_t memSize){
    
    int shmFd;
    
    if( ( shmFd = shm_open(SHM_PATH, O_CREAT | O_RDWR, S_RWALL) ) == ERROR) {
        handle_error("Shm_open failed");
    }

    if( ftruncate(shmFd, memSize) == ERROR ) {
        handle_error("Ftruncate failed");
    }

    void * ptr;
    if( ( ptr = mmap(0, memSize, PROT_WRITE, MAP_SHARED, shmFd, 0) ) == MAP_FAILED) {
        handle_error("Mmap failed");
    }

    if( close(shmFd) == ERROR ) {
        handle_error("Close failed");
    }
     
    return (char *) ptr;
}

void closeShm(char* ptr, size_t memSize){

    if ( munmap(ptr, memSize)== ERROR ){
        handle_error("munmap failed");
    }
    
    if ( shm_unlink(SHM_PATH) == ERROR ){
        handle_error("shm_unlink failed");
    }
}

void closeSem(sem_t * sem){

    if( sem_close(sem) == ERROR ){
        handle_error("Sem_close failed");
    }

    if( sem_unlink(SEM_NAME) == ERROR ){
        handle_error("Sem_unlink failed");
    }
}

void terminateSlaves(process slaves [], int totalSlaves) {
    for(int i = 0; i < totalSlaves; i++) {
        if( close(slaves[i].inFd) == ERROR ) {
            handle_error("Close failed");
        }   

        if( wait(NULL) == ERROR ) {
            handle_error("Wait failed");
        }
    }
}