#ifndef INC_25_26_27_PIPE_OPERATIONS_H
#define INC_25_26_27_PIPE_OPERATIONS_H

#include <stdbool.h>
#include <stdio.h>

#define MSG_LENGTH_LIMIT (16 * 1024)

typedef struct message_t {
    char *data;
    size_t len;
} message_t;

bool write_into_file(int fd, message_t *message);

bool fwrite_into_pipe(FILE *pipe_fd, char *buffer, size_t len);

/*
 * reads as many bytes from file as possible,
 * but not more that MSG_LENGTH_LIMIT
 */
message_t *read_from_socket(int socket_fd);

char *read_from_file(int pipe_fd);

char *fread_from_pipe(FILE *pipe_fp);

#endif //INC_25_26_27_PIPE_OPERATIONS_H
