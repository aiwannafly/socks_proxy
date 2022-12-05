#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "io_operations.h"
#include "socks_messages.h"

#define FAIL (-1)
#define STOP_MESSAGE "exit\n"
#define BUFFER_SIZE (1024)

typedef struct args_t {
    bool valid;
    int proxy_server_port;
} args_t;

static bool extract_int(const char *buf, int *num) {
    if (NULL == buf || num == NULL) {
        return false;
    }
    char *end_ptr = NULL;
    *num = (int) strtol(buf, &end_ptr, 10);
    if (buf + strlen(buf) > end_ptr) {
        return false;
    }
    return true;
}

static args_t parse_args(int argc, char *argv[]) {
    args_t result;
    result.valid = false;
    if (argc < 1 + 1) {
        return result;
    }
    bool extracted = extract_int(argv[1], &result.proxy_server_port);
    if (!extracted) {
        return result;
    }
    result.valid = true;
    return result;
}

int main(int argc, char *argv[]) {
    args_t args = parse_args(argc, argv);
    if (!args.valid) {
        fprintf(stderr, "%s\n", "./prog <proxy port>");
        return EXIT_FAILURE;
    }
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd == FAIL) {
        perror("[CLIENT] Error in socket");
        return EXIT_FAILURE;
    }
    struct sockaddr_in serv_sockaddr;
    serv_sockaddr.sin_family = AF_INET;
    serv_sockaddr.sin_port = htons(args.proxy_server_port);
    char *ip_address = "127.0.0.1";
    if (inet_pton(AF_INET, ip_address, &serv_sockaddr.sin_addr) == FAIL) {
        perror("[CLIENT] Error in inet_pton");
        return EXIT_FAILURE;
    }
    int return_value = connect(socket_fd, (struct sockaddr *) &serv_sockaddr, sizeof(serv_sockaddr));
    if (return_value == FAIL) {
        perror("[CLIENT] Error in connect");
        goto FINISH;
    }
    message_t *greeting_message = create_default_client_greeting_message();
    if (greeting_message == NULL) {
        perror("[CLIENT] Error in create_default_client_greeting_message");
        goto FINISH;
    }
    bool written = write_into_file(socket_fd, greeting_message);
    free(greeting_message->data);
    free(greeting_message);
    if (!written) {
        perror("[CLIENT] Error in write_into_file");
        goto FINISH;
    }
    message_t *server_choice = read_from_socket(socket_fd);
    if (NULL == server_choice) {
        perror("[CLIENT] Error in read");
        goto FINISH;
    }
    char choice = parse_server_choice(server_choice, true);
    free(server_choice->data);
    free(server_choice);
    if (choice == FAIL) {
        perror("[CLIENT] Error parse_server_choice");
        goto FINISH;
    }
    conn_request_info_t request = {
            .address_type = IPV4_TYPE,
            .dest_port = 5010,
            .command_code = 1,
            .dest_address = "127.0.0.1"
    };
    message_t *request_message = create_conn_request_message(&request);
    if (request_message == NULL) {
        perror("[CLIENT] Error in write_conn_request_message");
        goto FINISH;
    }
    printf("[CLIENT] Trying to connect to %s %d using proxy with port %d\n", ip_address, request.dest_port,
           args.proxy_server_port);
    written = write_into_file(socket_fd, request_message);
    free(request_message->data);
    free(request_message);
    if (!written) {
        perror("[CLIENT] Error in write_into_file");
        goto FINISH;
    }
    message_t *reply_from_server = read_from_socket(socket_fd);
    if (NULL == reply_from_server) {
        perror("[CLIENT] Error in read");
        goto FINISH;
    }
    server_response_t *response = parse_response_message(reply_from_server, true);
    free(reply_from_server->data);
    free(reply_from_server);
    if (NULL == response) {
        perror("[CLIENT] Error in parse");
        goto FINISH;
    }
    if (response->status_code != 0) {
        fprintf(stderr, "Connection not established, error code: %d\n", response->status_code);
        free(response);
        goto FINISH;
    }
    printf("[CLIENT] Connection confirmed by proxy\n");
    char buffer[BUFFER_SIZE];
    while (true) {
        printf("--> ");
        char *input = fgets(buffer, BUFFER_SIZE, stdin);
        if (NULL == input) {
            perror("[CLIENT] Error in fgets");
            break;
        }
        message_t message = {
                .data = buffer,
                .len = strlen(buffer)
        };
        written = write_into_file(socket_fd, &message);
        if (!written) {
            perror("[CLIENT] Error in write");
            break;
        }
        if (strcmp(STOP_MESSAGE, input) == 0) {
            break;
        }
        reply_from_server = read_from_socket(socket_fd);
        if (NULL == reply_from_server) {
            perror("[CLIENT] Error in read");
            continue;
        }
        if (reply_from_server->len == 0) {
            printf("[CLIENT] Received empty reply from server\n");
            free(reply_from_server);
            continue;
        }
        printf("Reply: %s", reply_from_server->data);
        free(reply_from_server);
    }
    FINISH:
    {
        return_value = close(socket_fd);
        if (return_value == FAIL) {
            perror("error in close");
        }
        return EXIT_SUCCESS;
    }
}
