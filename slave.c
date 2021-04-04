// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include "slave.h"

void solveTask(char const* task);
void waitTasks();

int main(int argc, char const *argv[]){
    
    if( setvbuf(stdout, NULL, _IONBF, 0) ){
        handle_error("Setvbuf failed");
    } 

    for (int i = 1; i < argc; i++){
        solveTask(argv[i]);
    }

    waitTasks();
    
    return 0;
}

void waitTasks(){
    
    char newTask[MAX_BUFF + 1] = {0}; 
    int count = 0;
    
    while ( ( count = read(STDIN, newTask, MAX_BUFF) ) != READ_END){
        if(count == ERROR){
            handle_error("Read failed");
        }
        
        newTask[count] = 0;
        solveTask(newTask);
    }
}

void solveTask(char const* task){

    char command_line[MAX_BUFF] = {0};
    sprintf(command_line, "%s %s | grep -o -e \"Number of.*[0-9]\\+\" -e \"CPU time.*\" -e \".*SATISFIABLE\"", SAT_SOLVER, task);
    
    FILE* outputStream;
    if( (outputStream = popen(command_line, "r") ) == NULL){
        handle_error("Popen failed");
    }

    char line[MAX_BUFF] = {0};
	fread(line, sizeof(char), MAX_BUFF, outputStream);
    if( ferror(outputStream) ) {
        fprintf(stderr, "%s\n", "Fread failed");
        exit(EXIT_FAILURE);
    }

    printf("PID:%d\nFile:%s\n%s\t", getpid(), task, line);

    if(pclose(outputStream) == -1){
        handle_error("Pclose failed");
    }
}