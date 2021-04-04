#ifndef ERROR_H
#define ERROR_H

#define ERROR -1

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

#endif