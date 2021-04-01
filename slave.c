#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include "errorCons.h"

#define SAT_SOLVER "minisat"
#define MAX_BUFF 1024
#define READ_END 0
#define STDIN 0
#define STDOUT 1


void solveTask(char const* task);

void waitTasks();

int main(int argc, char const *argv[]){
    
    setvbuf(stdout, NULL, _IONBF, 0); //NO SE PARA QUE ES ESTO LPMMMMMMMMMMMM

    for (int i = 1; i < argc; i++)
        solveTask(argv[i]);

    //terminó las primeras tareas

    waitTasks();
    
    return 0;
}

void waitTasks(){
    
    char newTask[MAX_BUFF] = {0}; 
    int count = 0;
    
    while ( ( count = read(STDIN, newTask, MAX_BUFF) ) != READ_END){

        //fprintf(file, newTask);

        if(count == -1){
            exit(-1);
            //mas manejo de errores
        }
        
        newTask[count] = 0;
        solveTask(newTask);
        
    }
}
void solveTask(char const* task){

    char command_line[MAX_BUFF] = {0};
    sprintf(command_line, "%s %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", SAT_SOLVER, task);
    
    FILE* outputStream = popen(command_line, "r");
    if(outputStream == NULL){
        //manejo de errores, esto despues se cambia
        exit(1);
    }

    char line[MAX_BUFF] = {0};
	fread(line, sizeof(char), MAX_BUFF,outputStream);

    //Open semaforo
    printf("PID:%d\nFile:%s\n%s\t", getpid(), task, line);

    //write(STDOUT, line, strlen(line)); 

    //Close semaforo  

    if(pclose(outputStream) == -1){
        //mas manejo de errores
    }

}
