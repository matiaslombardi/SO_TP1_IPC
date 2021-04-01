#define ARGUMENTS_ERR 1
#define ERROR -1

#define handle_error(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)