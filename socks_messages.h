#ifndef PROXY_SERVER_SOCKS_MESSAGES_H
#define PROXY_SERVER_SOCKS_MESSAGES_H

#include "io_operations.h"

/*
 * This file is used to parse SOCKS version 5 messages
 */

#define SOCKS_VERSION (5)
#define IPV4_TYPE (1)
#define DOMAIN_TYPE (3)
#define ADDR_BUFFER_SIZE (256)
#define CONN_REFUSED (5)
#define UNREACHABLE (3)
#define GENERAL_ERROR (1)
#define MAX_AUTHS_COUNT (16)
#define NO_METHODS_ACCEPTED (0xFF)

typedef struct conn_request_info_t {
    char dest_address[ADDR_BUFFER_SIZE];
    char address_type;
    int dest_port;
    char command_code;
} conn_request_info_t;

typedef struct server_response_t {
    char bind_address[ADDR_BUFFER_SIZE];
    char address_type;
    int bind_port;
    char status_code;
} server_response_t;

typedef struct client_greeting_t {
    char auths_count;
    char auths[MAX_AUTHS_COUNT];
} client_greeting_t;

message_t *create_server_choice_message(char choice);

// creates default greeting with no authentication
message_t *create_default_client_greeting_message();

message_t *create_conn_request_message(const conn_request_info_t *info);

message_t *create_server_response_message(server_response_t *response);

conn_request_info_t *parse_conn_request_message(const message_t *message, bool allow_print_error);

server_response_t *parse_response_message(const message_t *message, bool allow_print_error);

client_greeting_t *parse_client_greeting(const message_t *message, bool allow_print_error);

char parse_server_choice(const message_t *message, bool allow_print_error);

#endif //PROXY_SERVER_SOCKS_MESSAGES_H
