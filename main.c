#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>

#include "io_operations.h"

#define FAIL (-1)
#define SUCCESS (0)

//message_t *read_from_socket(int socket_fd) {
//    size_t capacity = DEFAULT_BUFFER_SIZE;
//    char *buffer = malloc(capacity + 1);
//    if (buffer == NULL) {
//        return NULL;
//    }
//    size_t offset = 0;
//    size_t portion = DEFAULT_BUFFER_SIZE;
//    message_t *message = (message_t *) malloc(sizeof(*message));
//    if (NULL == message) {
//        free(buffer);
//        return NULL;
//    }
//    while (true) {
//        if (offset + portion > capacity) {
//            capacity *= 2;
//            char *temp = realloc(buffer, capacity);
//            if (NULL == temp) {
//                free(buffer);
//                return NULL;
//            }
//            buffer = temp;
//        }
//        long read_bytes = read(socket_fd, buffer + offset, portion);
//        if (FAIL == read_bytes) {
//            if (errno == EINTR) {
//                continue;
//            } else if (errno == EAGAIN) {
//                fprintf(stderr, "EAGAIN\n");
//                buffer[offset] = '\0';
//                message->data = buffer;
//                message->len = offset;
//                return message;
//            } else {
//                free(buffer);
//                free(message);
//                return NULL;
//            }
//        }
//        offset += read_bytes;
//        if (0 == read_bytes || offset >= MSG_LENGTH_LIMIT) {
//            break;
//        }
//    }
//    buffer[offset] = '\0';
//    message->data = buffer;
//    message->len = offset;
//    return message;
//}

int main() {
    int rfd = open("big_text.txt", R_OK);
    int wfd = open("big_text2", W_OK);
    if (rfd == FAIL) {
        perror("error in open()");
        return FAIL;
    }
    if (wfd == FAIL) {
        perror("error in open()");
        close(rfd);
        return FAIL;
    }
    size_t total_len = 0;
    while (true) {
        message_t *part = read_from_socket(rfd);
        if (part == NULL) {
            perror("error in read_from_socket()");
            close(rfd);
            close(wfd);
            return FAIL;
        }
        total_len += part->len;
        printf("part len: %zu\n", part->len);
        if (part->len == 0) {
            free(part->data);
            free(part);
            break;
        }
        bool written = write_into_file(wfd, part);
        free(part->data);
        free(part);
        if (!written) {
            perror("error in write_into_file()");
            break;
        }
    }
    printf("total len: %zu\n", total_len);
    close(rfd);
    close(wfd);
    return SUCCESS;
}
