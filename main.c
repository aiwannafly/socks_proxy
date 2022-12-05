#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "socks_messages.h"

int main() {
    conn_request_info_t info = {
            .dest_port = 5010,
            .dest_address = "127.0.0.1",
            .address_type = IPV4_TYPE,
            .command_code = 1
    };
    message_t *message = write_conn_request_message(&info);
    if (message == NULL) {
        fprintf(stderr, "could not parse request info\n");
        return -1;
    }
    printf("len: %zu\n", message->len);
    conn_request_info_t *decoded = parse_conn_request_message(message, true);
    if (decoded == NULL) {
        fprintf(stderr, "could not decode request info\n");
        free(message);
        return -1;
    }
    printf("port = %d, addr = %s, type = %d, cmd_code = %d\n",
           decoded->dest_port, decoded->dest_address, decoded->address_type, decoded->command_code);
    free(message);
    free(decoded);
    return 0;
}
