#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>
#include "errorCons.h"

int main(int argc, char const *argv[])
{
    if(argc < 2) {
        fprintf(stderr,"%s\n","Incorrect amount of arguments");
        exit(ARGUMENTS_ERR);
    }
    return 0;
}
