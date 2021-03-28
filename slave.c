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

void solveTask(char const* task);

int main(int argc, char const *argv[]){
    //semaforo open
    if(argc < 2) {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(ARGUMENTS_ERR);
    }
    
    for (int i = 1; i < argc; i++)
        solveTask(argv[i]);
    //semaforo close
    return 0;
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
    //printf("%s", line);
    write(1, line, strlen(line)); 

    //Close semaforo  

    if(pclose(outputStream) == -1){
        //mas manejo de errores
    }
    return;
}
