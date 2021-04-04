#ifndef SLAVE_H
#define SLAVE_H

#define _POSIX_C_SOURCE 2

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <wait.h>

#include "errorCons.h"

#define SAT_SOLVER "minisat"
#define MAX_BUFF 4096
#define READ_END 0
#define STDIN 0
#define STDOUT 1

#endif