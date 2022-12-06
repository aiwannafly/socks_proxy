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
#include <assert.h>
#include <fcntl.h>

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

#define NEW_CLIENT (0)
#define PASSED_GREETING (1)
#define PASSED_SEND_REQUEST (2)
#define REJECTED (3)
#define SERVER (4)
#define WAIT_FOR_CONNECT (5)

int signal_pipe[2];

static int max(int a, int b);

typedef struct args_t {
    bool valid;
    int proxy_server_port;
    bool print_allowed;
} args_t;

typedef struct proxy_t {
    int max_fd;
    fd_set read_wait_set;
    fd_set write_wait_set;
    /* This table matches client socket of a proxy server
    * and client socket of a main server */
    int translation_table[MAX_CLIENTS_COUNT * 2 + 3];
    int status_table[MAX_CLIENTS_COUNT * 2 + 3];
    /*
     * When we need to send a message to socket, we
     * must add it to selector and put message here
     */
    message_t *message_queue[MAX_CLIENTS_COUNT * 2 + 3];
    bool has_message_to_send[MAX_CLIENTS_COUNT * 2 + 3];
    bool print_allowed;
} proxy_t;

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
    write_all(signal_pipe[WRITE_PIPE_END], &terminate);
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

static int handle_new_connection(int proxy_socket, proxy_t *proxy) {
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
    FD_SET(new_client_fd, &proxy->read_wait_set);
    if (new_client_fd > proxy->max_fd) {
        proxy->max_fd = new_client_fd;
    }
    proxy->has_message_to_send[new_client_fd] = false;
    return SUCCESS;
}

static void close_connection(int fd, proxy_t *proxy) {
    // connection was closed
    int return_value = close(fd);
    if (return_value == FAIL) {
        perror("[PROXY] Error in close");
    }
    if (proxy->print_allowed) printf("[PROXY] Closed connection %d\n", fd);
    FD_CLR(fd, &proxy->read_wait_set);
    if (fd == proxy->max_fd) {
        proxy->max_fd--;
    }
    if (proxy->translation_table[fd] != 0) {
        return_value = close(proxy->translation_table[fd]);
        if (return_value == FAIL) {
            perror("[PROXY] Error in close");
        }
        if (proxy->print_allowed) printf("[PROXY] Closed connection %d\n", proxy->translation_table[fd]);
        FD_CLR(proxy->translation_table[fd], &proxy->read_wait_set);
        if (proxy->translation_table[fd] == proxy->max_fd) {
            proxy->max_fd--;
        }
        proxy->translation_table[proxy->translation_table[fd]] = 0;
        proxy->translation_table[fd] = 0;
    }
}

static void put_message_into_queue(int fd, proxy_t *proxy, message_t *message) {
    if (proxy->has_message_to_send[fd] && proxy->message_queue[fd] != NULL) {
        free(proxy->message_queue[fd]->data);
        free(proxy->message_queue[fd]);
    }
    proxy->message_queue[fd] = message;
    FD_SET(fd, &proxy->write_wait_set);
    proxy->max_fd = max(proxy->max_fd, fd);
    proxy->has_message_to_send[fd] = true;
}

static int handle_greeting(int fd, proxy_t *proxy, message_t *greeting_msg) {
    assert(greeting_msg);
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
    char choice = WITHOUT_AUTH;
    if (!acceptable) {
        choice = NO_METHODS_ACCEPTED;
    }
    message_t *choice_message = create_server_choice_message(choice);
    if (choice_message == NULL) {
        fprintf(stderr, "[PROXY] could not create choice message\n");
        return FAIL;
    }
    put_message_into_queue(fd, proxy, choice_message);
    if (proxy->print_allowed) printf("[PROXY] Pushed greeting into queue, fd = %d\n", fd);
    if (acceptable) {
        return SUCCESS;
    }
    return FAIL;
}

static int connect_to_remote(int sd, proxy_t *proxy) {
    FD_CLR(sd, &proxy->write_wait_set);
    proxy->status_table[sd] = NEW_CLIENT;
    int opt = fcntl(sd, F_GETFL, NULL);
    if (opt < 0) {
        close(sd);
        return FAIL;
    }
    socklen_t len = sizeof(opt);
    int return_code = getsockopt(sd, SOL_SOCKET, SO_ERROR, &opt, &len);
    if (return_code < 0) {
        close(sd);
        return FAIL;
    }
    if (opt != SUCCESS) {
        errno = opt;
        close(sd);
        return FAIL;
    }
    proxy->status_table[sd] = SERVER;
    return SUCCESS;
}

static int start_connecting(char *serv_ipv4_address, int port, proxy_t *proxy) {
    if (port < 0 || port >= 65536) {
        return FAIL;
    }
    int sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == FAIL) {
        return FAIL;
    }
    struct sockaddr_in serv_sockaddr;
    serv_sockaddr.sin_family = AF_INET;
    serv_sockaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, serv_ipv4_address, &serv_sockaddr.sin_addr) == FAIL) {
        close(sd);
        return FAIL;
    }
    int opt = fcntl(sd, F_GETFL, NULL);
    if (opt < 0) {
        close(sd);
        return FAIL;
    }
    int return_code = fcntl(sd, F_SETFL, opt | O_NONBLOCK);
    if (return_code < 0) {
        close(sd);
        return FAIL;
    }
    return_code = connect(sd, (const struct sockaddr *) &serv_sockaddr, sizeof(serv_sockaddr));
    if (return_code < 0) {
        if (errno == EINPROGRESS) {
            FD_SET(sd, &proxy->write_wait_set);
            proxy->status_table[sd] = WAIT_FOR_CONNECT;
            proxy->max_fd = max(proxy->max_fd, sd);
            return sd;
        }
        return FAIL;
    }
    proxy->status_table[sd] = SERVER;
    return sd;
}

static int handle_conn_request(int fd, proxy_t *proxy, message_t *message) {
    assert(proxy);
    assert(message);
    conn_request_info_t *info = parse_conn_request_message(message, true);
    if (info == NULL) {
        fprintf(stderr, "[PROXY] could not parse request message\n");
        return FAIL;
    }
    if (proxy->print_allowed) printf("[PROXY] Got request to connect to %s %d\n", info->dest_address, info->dest_port);
    int server_fd = start_connecting(info->dest_address, info->dest_port, proxy);
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
        close_connection(fd, proxy);
        return FAIL;
    }
    put_message_into_queue(fd, proxy, response_msg);
    if (server_fd != FAIL) {
        int return_value = set_nonblocking(server_fd);
        if (return_value == FAIL) {
            close_connection(fd, proxy);
            close(server_fd);
            return FAIL;
        }
        if (proxy->print_allowed) printf("[PROXY] Connected\n");
        proxy->translation_table[fd] = server_fd;
        proxy->translation_table[server_fd] = fd;
        FD_SET(server_fd, &proxy->read_wait_set);
        if (server_fd > proxy->max_fd) {
            proxy->max_fd = server_fd;
        }
    } else {
        close_connection(fd, proxy);
    }
    return SUCCESS;
}

/*
 * returns FAIL, SUCCESS OR TERMINATE codes
 */
static int handle_new_message(int fd, proxy_t *proxy) {
    if (fd == signal_pipe[READ_PIPE_END]) {
        char *message = read_from_file(fd);
        if (strcmp(message, TERMINATE_COMMAND) == 0) {
            free(message);
            return TERMINATE;
        }
    }
    message_t *message = read_all(fd);
    if (NULL == message) {
        perror("[PROXY] Error in read");
        return FAIL;
    }
    if (message->len == 0) {
        free(message->data);
        free(message);
        close_connection(fd, proxy);
        return SUCCESS;
    }
    // here we got a message from a client
    // we should check whether he established connection or not
    if (proxy->status_table[fd] == NEW_CLIENT) {
        int return_value = handle_greeting(fd, proxy, message);
        free(message->data);
        free(message);
        if (return_value == SUCCESS) {
            if (proxy->print_allowed) printf("[PROXY] Greeting passed successfully\n");
            proxy->status_table[fd] = PASSED_GREETING;
        } else {
            if (proxy->print_allowed) printf("[PROXY] Greeting not passed\n");
            close_connection(fd, proxy);
            proxy->status_table[fd] = REJECTED;
        }
        return return_value;
    } else if (proxy->status_table[fd] == PASSED_GREETING) {
        int return_value = handle_conn_request(fd, proxy, message);
        free(message->data);
        free(message);
        if (return_value == SUCCESS) {
            proxy->status_table[fd] = PASSED_SEND_REQUEST;
        } else {
            close_connection(fd, proxy);
            proxy->status_table[fd] = REJECTED;
        }
        return return_value;
    } else if (proxy->status_table[fd] == REJECTED) {
        free(message->data);
        free(message);
        close_connection(fd, proxy);
        return FAIL;
    }
    if (proxy->print_allowed) printf("[PROXY] Received from %d:\n%s\n\nLength: %zu\n", fd, message->data, message->len);
    if (proxy->print_allowed) printf("[PROXY] Pushed message to a queue for %d\n", proxy->translation_table[fd]);
    proxy->status_table[proxy->translation_table[fd]] = SERVER;
    put_message_into_queue(proxy->translation_table[fd], proxy, message);
    return SUCCESS;
}

static int max(int a, int b) {
    if (a >= b) {
        return a;
    }
    return b;
}

static int send_message(int fd, proxy_t *proxy, message_t *message) {
    bool written = write_all(fd, message);
    size_t len = message->len;
    free(message->data);
    free(message);
    proxy->message_queue[fd] = NULL;
    proxy->has_message_to_send[fd] = false;
    if (!written) {
        perror("[PROXY] Error in write_all()");
        return FAIL;
    } else {
        if (proxy->print_allowed) printf("[PROXY] sent %zu bytes\n", len);
    }
    return SUCCESS;
}

int main(int argc, char *argv[]) {
    args_t args = parse_args(argc, argv);
    if (!args.valid) {
        fprintf(stderr, "%s\n", USAGE_GUIDE);
        return EXIT_FAILURE;
    }
    proxy_t proxy;
    memset(proxy.translation_table, 0x00, MAX_CLIENTS_COUNT * 2 + 3);
    memset(proxy.message_queue, 0x00, (MAX_CLIENTS_COUNT * 2 + 3));
    memset(proxy.has_message_to_send, false, (MAX_CLIENTS_COUNT * 2 + 3));
    memset(proxy.status_table, NEW_CLIENT, MAX_CLIENTS_COUNT * 2 + 3);
    if (args.print_allowed) {
        proxy.print_allowed = true;
    }
    int return_value = init_signal_handlers();
    if (return_value == FAIL) {
        fprintf(stderr, "[PROXY] Error in init_signal_handlers()\n");
        return EXIT_FAILURE;
    }
    int proxy_socket = init_and_bind_proxy_socket(args);
    if (proxy_socket == FAIL) {
        fprintf(stderr, "[PROXY] Error in init_and_bind_proxy_socket()\n");
        return EXIT_FAILURE;
    }
    return_value = listen(proxy_socket, MAX_CLIENTS_COUNT);
    if (return_value == FAIL) {
        perror("[PROXY] Error in listen");
        return_value = close(proxy_socket);
        if (return_value == FAIL) {
            perror("[PROXY] Error in close");
        }
        return EXIT_FAILURE;
    }
    FD_ZERO(&proxy.write_wait_set);
    FD_ZERO(&proxy.read_wait_set);
    fd_set constant_read_set;
    fd_set constant_write_set;
    proxy.max_fd = proxy_socket;
    FD_SET(signal_pipe[READ_PIPE_END], &proxy.read_wait_set);
    FD_SET(proxy_socket, &proxy.read_wait_set); // add listen_fd to our set
    struct timeval timeout = {
            .tv_sec = WAIT_TIME,
            .tv_usec = 0
    };
    bool shutdown = false;
    if (proxy.print_allowed) printf("[PROXY] Running...\n");
    while (shutdown == false) {
        if (proxy.print_allowed) printf("[PROXY] Waiting on select, max_fd = %d\n", proxy.max_fd);
        memcpy(&constant_read_set, &proxy.read_wait_set, sizeof(proxy.read_wait_set));
        memcpy(&constant_write_set, &proxy.write_wait_set, sizeof(proxy.write_wait_set));
        return_value = select(proxy.max_fd + 1, &constant_read_set, &constant_write_set, NULL, &timeout);
        if (return_value == FAIL || return_value == TIMEOUT_CODE) {
            if (errno != EINTR) perror("[PROXY] Error in select");
            if (return_value == TIMEOUT_CODE) fprintf(stderr, "[PROXY] Select timed out. End program.\n");
            break;
        }
        int desc_ready = return_value;
        for (int fd = 0; fd <= proxy.max_fd && desc_ready > 0; ++fd) {
            if (FD_ISSET(fd, &constant_write_set)) {
                desc_ready -= 1;
                if (proxy.print_allowed) printf("[PROXY] Ready to send message to %d\n", fd);
                if (proxy.status_table[fd] == WAIT_FOR_CONNECT) {
                    return_value = connect_to_remote(fd, &proxy);
                    if (return_value == FAIL) {
                        fprintf(stderr, "[PROXY] failed to connect\n");
                    } else {
                        if (proxy.print_allowed) printf("[PROXY] Connected\n");
                    }
                } else {
                    FD_CLR(fd, &proxy.write_wait_set);
                    message_t *message = proxy.message_queue[fd];
                    if (message == NULL) {
                        fprintf(stderr, "[PROXY] NULL message to send\n");
                    } else {
                        send_message(fd, &proxy, message);
                    }
                }
            }
            if (FD_ISSET(fd, &constant_read_set)) {
                desc_ready -= 1;
                if (fd == proxy_socket) {
                    if (proxy.print_allowed) fprintf(stderr, "[PROXY] handle new connection... %d\n", fd);
                    return_value = handle_new_connection(proxy_socket, &proxy);
                    if (return_value == FAIL) {
                        shutdown = true;
                        break;
                    }
                } else {
                    if (proxy.print_allowed) fprintf(stderr, "[PROXY] handle new message...\n");
                    return_value = handle_new_message(fd, &proxy);
                    if (return_value == TERMINATE) {
                        goto FINISH;
                    }
                }
            }
        }
    }
    FINISH:
    {
        for (int i = 0; i < MAX_CLIENTS_COUNT * 2 + 3; i++) {
            if (proxy.has_message_to_send[i]) {
                message_t *message = proxy.message_queue[i];
                if (message == 0x00) continue;
                free(message->data);
                free(message);
            }
        }
        if (proxy.print_allowed) printf("\n[PROXY] Shutdown...\n");
        for (int fd = 0; fd <= proxy.max_fd; ++fd) {
            if (FD_ISSET(fd, &proxy.read_wait_set) || FD_ISSET(fd, &proxy.write_wait_set)) {
                return_value = close(fd);
                if (return_value == FAIL) {
                    perror("=== Error in close");
                }
            }
        }
    }
}
