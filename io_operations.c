#include "io_operations.h"

#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define FAIL (-1)
#define DEFAULT_BUFFER_SIZE (128)

bool write_all(int fd, message_t *message) {
    if (NULL == message) {
        return false;
    }
    if (NULL == message->data) {
        return false;
    }
    size_t written_bytes = 0;
    size_t left_to_write = message->len;
    while (true) {
        ssize_t count = write(fd, message->data + written_bytes, left_to_write);
        if (count == FAIL) {
            return false;
        }
        written_bytes += count;
        if (written_bytes == message->len) {
            return true;
        }
        left_to_write -= count;
    }
}

bool fwrite_into_pipe(FILE *pipe_fd, char *buffer, size_t len) {
    if (NULL == buffer || NULL == pipe_fd) {
        return false;
    }
    size_t written_bytes = 0;
    size_t left_to_write = len;
    while (true) {
        size_t count = fwrite(buffer + written_bytes, sizeof(char),  left_to_write, pipe_fd);
        if (ferror(pipe_fd)) {
            return false;
        }
        written_bytes += count;
        if (written_bytes == len) {
            return true;
        }
        left_to_write -= count;
    }
}

message_t *read_all(int socket_fd) {
    size_t capacity = DEFAULT_BUFFER_SIZE;
    char *buffer = malloc(capacity + 1);
    if (buffer == NULL) {
        return NULL;
    }
    size_t offset = 0;
    size_t portion = DEFAULT_BUFFER_SIZE;
    while (true) {
        if (offset + portion > capacity) {
            capacity *= 2;
            char *temp = realloc(buffer, capacity);
            if (NULL == temp) {
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }
        long read_bytes = read(socket_fd, buffer + offset, portion);
        if (FAIL == read_bytes) {
            if (errno == EINTR) {
                continue;
            } else if (errno == EAGAIN) {
                buffer[offset] = '\0';
                message_t *message = (message_t *) malloc(sizeof(*message));
                if (NULL == message) {
                    free(buffer);
                    return NULL;
                }
                message->data = buffer;
                message->len = offset;
                return message;
            } else {
                free(buffer);
                return NULL;
            }
        }
        if (0 == read_bytes || offset >= MSG_LENGTH_LIMIT) {
            offset += read_bytes;
            break;
        } else {
            offset += read_bytes;
            if (offset % portion != 0) {
                break;
            }
        }
    }
    buffer[offset] = '\0';
    message_t *message = (message_t *) malloc(sizeof(*message));
    if (NULL == message) {
        free(buffer);
        return NULL;
    }
    message->data = buffer;
    message->len = offset;
    return message;
}

char *read_from_file(int pipe_fd) {
    size_t capacity = DEFAULT_BUFFER_SIZE;
    char *buffer = malloc(capacity);
    size_t offset = 0;
    size_t portion = DEFAULT_BUFFER_SIZE;
    while (true) {
        printf("offset: %zu\n", offset);
        if (offset + portion > capacity) {
            capacity *= 2;
            char *temp = realloc(buffer, capacity);
            if (NULL == temp) {
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }
        if (buffer[offset] == 0) {
            break;
        }
        long read_bytes = read(pipe_fd, buffer + offset, portion);
        if (FAIL == read_bytes) {
            if (errno == EINTR) {
                continue;
            } else {
                free(buffer);
                return NULL;
            }
        }
        if (0 == read_bytes) {
            break;
        } else {
            offset += read_bytes;
        }
    }
    buffer[offset] = '\0';
    return buffer;
}

char *fread_from_pipe(FILE *pipe_fp) {
    size_t capacity = DEFAULT_BUFFER_SIZE;
    char *buffer = malloc(capacity);
    size_t offset = 0;
    size_t portion = DEFAULT_BUFFER_SIZE;
    while (true) {
        if (offset + portion > capacity) {
            capacity *= 2;
            char *temp = realloc(buffer, capacity);
            if (NULL == temp) {
                free(buffer);
                return NULL;
            }
            buffer = temp;
        }
        unsigned long read_bytes = fread(buffer + offset, sizeof(char), portion, pipe_fp);
        if (ferror(pipe_fp)) {
            free(buffer);
            return NULL;
        }
        if (0 == read_bytes) {
            break;
        } else {
            offset += read_bytes;
        }
    }
    buffer[offset] = '\0';
    return buffer;
}
