#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <unistd.h>

#include "io_operations.h"
#include "socket_operations.h"
#include "pipe_operations.h"
#include "socks_messages.h"

#define SUCCESS (0)
#define FAIL (-1)
#define TERMINATE (1)
#define MAX_CLIENTS_COUNT (510)
#define WAIT_TIME (3 * 60)
#define TIMEOUT_CODE (0)
#define REQUIRED_ARGC (1 + 1)
#define USAGE_GUIDE "usage: ./prog <proxy_port>"
#define READ_PIPE_END (0)
#define WRITE_PIPE_END (1)
#define TERMINATE_COMMAND "stop"
#define CONNECT_TIMEOUT (5)

#define NEW_CLIENT (0)
#define PASSED_GREETING (1)
#define PASSED_SEND_REQUEST (2)
#define REJECTED (3)
#define SERVER (4)

int signal_pipe[2];
bool print_allowed = false;
int status_table[MAX_CLIENTS_COUNT * 2 + 3];

typedef struct args_t {
    bool valid;
    int proxy_server_port;
    bool print_allowed;
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
    if (argc < REQUIRED_ARGC) {
        return result;
    }
    bool extracted = extract_int(argv[1], &result.proxy_server_port);
    if (!extracted) {
        return result;
    }
    result.print_allowed = false;
    if (argc == REQUIRED_ARGC + 1) {
        if (strcmp(argv[2], "-p") == 0) {
            result.print_allowed = true;
        }
    }
    result.valid = true;
    return result;
}

static int init_and_bind_proxy_socket(args_t args) {
    int proxy_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (proxy_socket == FAIL) {
        perror("[PROXY] Error in socket");
        return FAIL;
    }
    int return_value = set_reusable(proxy_socket);
    if (return_value == FAIL) {
        close(proxy_socket);
        fprintf(stderr, "[PROXY] Failed to make socket reusable\n");
        return FAIL;
    }
    return_value = set_nonblocking(proxy_socket);
    if (return_value == FAIL) {
        close(proxy_socket);
        fprintf(stderr, "[PROXY] Failed to make socket nonblocking\n");
        return FAIL;
    }
    struct sockaddr_in proxy_sockaddr;
    proxy_sockaddr.sin_family = AF_INET;
    proxy_sockaddr.sin_addr.s_addr = INADDR_ANY;
    proxy_sockaddr.sin_port = htons(args.proxy_server_port);
    return_value = bind(proxy_socket, (struct sockaddr *) &proxy_sockaddr, sizeof(proxy_sockaddr));
    if (return_value < 0) {
        perror("[PROXY] Error in bind");
        return_value = close(proxy_socket);
        if (return_value == FAIL) {
            perror("[PROXY] Error in close");
        }
        return FAIL;
    }
    return proxy_socket;
}

static void handle_sigint_sigterm(__attribute__((unused)) int sig) {
    message_t terminate = {
            .data = TERMINATE_COMMAND,
            .len = strlen(TERMINATE_COMMAND)
    };
    write_into_file(signal_pipe[WRITE_PIPE_END], &terminate);
}

static int init_signal_handlers() {
    int return_value = pipe(signal_pipe);
    if (return_value == FAIL) {
        perror("[PROXY] Error in pipe()");
        return FAIL;
    }
    signal(SIGINT, handle_sigint_sigterm);
    signal(SIGTERM, handle_sigint_sigterm);
    return SUCCESS;
}

static int handle_new_connection(int proxy_socket, fd_set *read_set, int *max_sd) {
    int new_client_fd = accept(proxy_socket, NULL, NULL);
    if (new_client_fd == FAIL) {
        if (errno != EAGAIN) {
            perror("[PROXY] Error in accept. Shutdown server...");
        }
        return FAIL;
    }
    int return_value = set_nonblocking(new_client_fd);
    if (return_value == FAIL) {
        close(new_client_fd);
        return FAIL;
    }
    FD_SET(new_client_fd, read_set);
    if (new_client_fd > *max_sd) {
        *max_sd = new_client_fd;
    }
    return SUCCESS;
}

static void close_connection(int fd, int *translation_table, fd_set *read_set, int *max_sd) {
    // connection was closed
    int return_value = close(fd);
    if (return_value == FAIL) {
        perror("[PROXY] Error in close");
    }
    if (print_allowed) printf("[PROXY] Closed connection %d\n", fd);
    FD_CLR(fd, read_set);
    if (fd == *max_sd) {
        *max_sd -= 1;
    }
    if (translation_table[fd] != 0) {
        return_value = close(translation_table[fd]);
        if (return_value == FAIL) {
            perror("[PROXY] Error in close");
        }
        if (print_allowed) printf("[PROXY] Closed connection %d\n", translation_table[fd]);
        FD_CLR(translation_table[fd], read_set);
        if (translation_table[fd] == *max_sd) {
            *max_sd -= 1;
        }
        translation_table[translation_table[fd]] = 0;
        translation_table[fd] = 0;
    }
}

static int handle_greeting(int fd, message_t *greeting_msg) {
    client_greeting_t *greeting = parse_client_greeting(greeting_msg, true);
    if (greeting == NULL) {
        fprintf(stderr, "[PROXY] could not parse greeting message\n");
        return FAIL;
    }
    bool acceptable = false;
    for (int i = 0; i < greeting->auths_count; i++) {
        if (greeting->auths[i] == 0x00) {
            acceptable = true;
            break;
        }
    }
    free(greeting);
    char choice = 0x00;
    if (!acceptable) {
        choice = NO_METHODS_ACCEPTED;
    }
    message_t *choice_message = create_server_choice_message(choice);
    if (choice_message == NULL) {
        fprintf(stderr, "[PROXY] could not create choice message\n");
        return FAIL;
    }
    bool written = write_into_file(fd, choice_message);
    free(choice_message->data);
    free(choice_message);
    if (!written) {
        fprintf(stderr, "[PROXY] error in write_into_file\n");
        return FAIL;
    }
    if (acceptable) {
        return SUCCESS;
    }
    return FAIL;
}

static int handle_conn_request(int fd, int *translation_table, fd_set *read_set, int *max_sd, message_t *message) {
    conn_request_info_t *info = parse_conn_request_message(message, true);
    if (info == NULL) {
        fprintf(stderr, "[PROXY] could not parse request message\n");
        return FAIL;
    }
    if (print_allowed) printf("[PROXY] Got request to connect to %s %d\n", info->dest_address, info->dest_port);
    struct timeval timeout = {
            .tv_sec = CONNECT_TIMEOUT,
            .tv_usec = 0
    };
    int server_fd = connect_to_address(info->dest_address, info->dest_port, &timeout);
    char status_code = 0; // success
    if (server_fd == FAIL) {
        if (errno == ENETUNREACH) {
            status_code = UNREACHABLE;
        } else if (errno == ECONNREFUSED) {
            status_code = CONN_REFUSED;
        } else {
            status_code = GENERAL_ERROR;
        }
        fprintf(stderr, "[PROXY] failed to establish connection, code: %d\n", status_code);
    }
    int return_value = set_nonblocking(server_fd);
    if (return_value == FAIL) {
        close(server_fd);
        return FAIL;
    }
    if (print_allowed) printf("[PROXY] Connected\n");
    server_response_t response = {
            .status_code = status_code,
            .bind_port = info->dest_port,
            .address_type = info->address_type
    };
    strcpy(response.bind_address, info->dest_address);
    free(info);
    message_t *response_msg = create_server_response_message(&response);
    if (response_msg == NULL) {
        fprintf(stderr, "[PROXY] could not make response message\n");
        if (server_fd != FAIL) {
            close(server_fd);
        }
        return FAIL;
    }
    bool written = write_into_file(fd, response_msg);
    free(response_msg->data);
    free(response_msg);
    if (!written) {
        fprintf(stderr, "[PROXY] error in write_into_file()\n");
    }
    if (server_fd != FAIL) {
        translation_table[fd] = server_fd;
        translation_table[server_fd] = fd;
        FD_SET(server_fd, read_set);
        if (server_fd > *max_sd) {
            *max_sd = server_fd;
        }
        status_table[server_fd] = SERVER;
    }
    return SUCCESS;
}

/*
 * returns FAIL, SUCCESS OR TERMINATE codes
 */
static int handle_new_message(int fd, int *translation_table, fd_set *read_set, int *max_sd) {
    if (fd == signal_pipe[READ_PIPE_END]) {
        char *message = read_from_file(fd);
        if (strcmp(message, TERMINATE_COMMAND) == 0) {
            free(message);
            return TERMINATE;
        }
    }
    fprintf(stderr, "1\n");
    message_t *message = read_from_socket(fd);
    fprintf(stderr, "2\n");
    if (NULL == message) {
        perror("[PROXY] Error in read");
        return FAIL;
    }
    if (message->len == 0) {
        free(message->data);
        free(message);
        close_connection(fd, translation_table, read_set, max_sd);
        return SUCCESS;
    }
    // here we got a message from a client
    // we should check whether he established connection or not
    fprintf(stderr, "3\n");
    if (status_table[fd] == NEW_CLIENT) {
        int return_value = handle_greeting(fd, message);
        free(message->data);
        free(message);
        if (return_value == SUCCESS) {
            printf("[PROXY] Greeting passed successfully\n");
            status_table[fd] = PASSED_GREETING;
        } else {
            printf("[PROXY] Greeting not passed\n");
            close_connection(fd, translation_table, read_set, max_sd);
            status_table[fd] = REJECTED;
        }
        return return_value;
    } else if (status_table[fd] == PASSED_GREETING) {
        int return_value = handle_conn_request(fd, translation_table, read_set, max_sd, message);
        free(message->data);
        free(message);
        if (return_value == SUCCESS) {
            status_table[fd] = PASSED_SEND_REQUEST;
        } else {
            close_connection(fd, translation_table, read_set, max_sd);
            status_table[fd] = REJECTED;
        }
        return return_value;
    }
    fprintf(stderr, "4\n");
    if (print_allowed) printf("[PROXY] Received from %d:\n%s\n\nLength: %zu\n", fd, message->data, message->len);
    bool sent = write_into_file(translation_table[fd], message);
    fprintf(stderr, "5\n");
    if (!sent) {
        perror("[PROXY] Error in write");
    }
    free(message->data);
    free(message);
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    memset(status_table, NEW_CLIENT, MAX_CLIENTS_COUNT * 2 + 3);
    args_t args = parse_args(argc, argv);
    if (!args.valid) {
        fprintf(stderr, "%s\n", USAGE_GUIDE);
        return EXIT_FAILURE;
    }
    if (args.print_allowed) {
        print_allowed = true;
    }
    int return_value = init_signal_handlers();
    if (return_value == FAIL) {
        fprintf(stderr, "[PROXY] Error in init_signal_handlers()\n");
        return EXIT_FAILURE;
    }
    /* This table matches client socket of a proxy server
     * and client socket of a main server */
    int *translation_table = (int *) calloc(MAX_CLIENTS_COUNT * 2 + 3, sizeof(*translation_table));
    if (translation_table == NULL) {
        fprintf(stderr, "[PROXY] Error in malloc()\n");
        return EXIT_FAILURE;
    }
    int proxy_socket = init_and_bind_proxy_socket(args);
    if (proxy_socket == FAIL) {
        fprintf(stderr, "[PROXY] Error in init_and_bind_proxy_socket()\n");
        free(translation_table);
        return EXIT_FAILURE;
    }
    return_value = listen(proxy_socket, MAX_CLIENTS_COUNT);
    if (return_value == FAIL) {
        perror("[PROXY] Error in listen");
        return_value = close(proxy_socket);
        if (return_value == FAIL) {
            perror("[PROXY] Error in close");
        }
        free(translation_table);
        return EXIT_FAILURE;
    }
    fd_set master_read_set;
    FD_ZERO(&master_read_set);
    int max_sd = proxy_socket;
    FD_SET(signal_pipe[READ_PIPE_END], &master_read_set);
    FD_SET(proxy_socket, &master_read_set); // add listen_fd to our set
    struct timeval timeout = {
            .tv_sec = WAIT_TIME,
            .tv_usec = 0
    };
    fd_set working_read_set;
    bool shutdown = false;
    if (print_allowed) printf("[PROXY] Running...\n");
    while (shutdown == false) {
        if (print_allowed) printf("[PROXY] Waiting on select...\n");
        memcpy(&working_read_set, &master_read_set, sizeof(master_read_set));
        return_value = select(max_sd + 1, &working_read_set, NULL, NULL, &timeout);
        if (return_value == FAIL) {
            if (errno != EINTR) {
                perror("[PROXY] Error in select");
            }
            break;
        }
        if (return_value == TIMEOUT_CODE) {
            fprintf(stderr, "[PROXY] Select timed out. End program.\n");
            break;
        }
        int desc_ready = return_value;
        for (int fd = 0; fd <= max_sd && desc_ready > 0; ++fd) {
            if (FD_ISSET(fd, &working_read_set)) {
                desc_ready -= 1;
                if (fd == proxy_socket) {
                    fprintf(stderr, "[PROXY] handle new connection... %d\n", fd);
                    return_value = handle_new_connection(proxy_socket, &master_read_set, &max_sd);
                    if (return_value == FAIL) {
                        shutdown = true;
                        break;
                    }
                } else {
                    fprintf(stderr, "[PROXY] handle new message...\n");
                    return_value = handle_new_message(fd, translation_table, &master_read_set, &max_sd);
                    if (return_value == TERMINATE) {
                        goto FINISH;
                    }
                }
            }
        }
    }
    FINISH:
    {
        free(translation_table);
        if (print_allowed) printf("\n[PROXY] Shutdown...\n");
        for (int sock_fd = 0; sock_fd <= max_sd; ++sock_fd) {
            if (FD_ISSET(sock_fd, &master_read_set)) {
                return_value = close(sock_fd);
                if (return_value == FAIL) {
                    perror("=== Error in close");
                }
            }
        }
    }
}
