#ifndef INC_25_26_27_PIPE_OPERATIONS_H
#define INC_25_26_27_PIPE_OPERATIONS_H

#include <stdbool.h>
#include <stdio.h>

bool write_into_pipe(int pipe_fd, char *buffer, size_t len);

bool fwrite_into_pipe(FILE *pipe_fd, char *buffer, size_t len);

char *read_from_pipe(int pipe_fd);

char *fread_from_pipe(FILE *pipe_fp);

#endif //INC_25_26_27_PIPE_OPERATIONS_H
