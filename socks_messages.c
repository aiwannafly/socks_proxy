#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdbool.h>

#include "socks_messages.h"

struct socks_packet_t {
    char address[ADDR_BUFFER_SIZE];
    char address_type;
    int port;
    char code;
};

static message_t *write_packet(struct socks_packet_t info) {
    size_t bytes_for_address = 0;
    if (info.address_type == IPV4_TYPE) {
        bytes_for_address = 4;
    } else if (info.address_type == DOMAIN_TYPE) {
        bytes_for_address = 1 + strlen(info.address);
    } else {
        return NULL;
    }
    size_t bytes_to_alloc = 1 + 1 + 1 + (1 + bytes_for_address) + 2;
    char *res = (char *) malloc((bytes_to_alloc + 1) * sizeof(*res));
    if (res == NULL) {
        return NULL;
    }
    size_t current_idx = 0;
    res[current_idx++] = SOCKS_VERSION;
    res[current_idx++] = info.code;
    res[current_idx++] = 0x00; // reserved byte
    res[current_idx++] = info.address_type;
    if (info.address_type == IPV4_TYPE) {
        struct in_addr addr;
        int return_code = inet_aton(info.address, &addr);
        if (return_code == 0) {
            free(res);
            return NULL;
        }
        sprintf(&res[current_idx], "%c%c%c%c",
                addr.s_addr & 255, (addr.s_addr >> 8) & 255,
                (addr.s_addr >> 16) & 255, (addr.s_addr >> 24) & 255);
        current_idx += 4;
    } else { // hostname
        uint addr_len = strlen(info.address);
        if (addr_len > 255) {
            free(res);
            return NULL;
        }
        res[current_idx++] = (char) addr_len;
        strcpy(&res[current_idx], info.address);
        current_idx += addr_len;
    }
    // 1024 * 2 = 2048; 2048 * 2 = 4196; 4196 * 2 = 8392; 8392
    if (info.port < 0 | info.port >= 65536) {
        free(res);
        return NULL;
    }
    uint16_t port_bytes = htons(info.port);
    sprintf(&res[current_idx], "%c%c", port_bytes & 255, (port_bytes >> 8) & 255);
    current_idx += 2;
    message_t *message = (message_t *) (malloc(sizeof(*message)));
    message->data = res;
    message->len = current_idx;
    return message;
}

/*
 * 	          VER	NAUTH	AUTH
    Byte count	1	1	variable
 */
message_t *create_default_client_greeting_message() {
    message_t *message = (message_t *) malloc(sizeof(*message));
    if (message == NULL) {
        return message;
    }
    message->len = 1 + 1 + 1;
    message->data = (char *) malloc(message->len + 1);
    if (message->data == NULL) {
        free(message);
        return NULL;
    }
    int current_idx = 0;
    message->data[current_idx++] = SOCKS_VERSION;
    message->data[current_idx++] = 1; // only 1 method supported
    message->data[current_idx++] = 0; // no authentication required
    return message;
}

/*
 * 	           VER	CAUTH
    Byte count	1	  1
 */
message_t *create_server_choice_message(char choice) {
    message_t *message = (message_t *) malloc(sizeof(*message));
    if (message == NULL) {
        return message;
    }
    message->len = 1 + 1;
    message->data = (char *) malloc(message->len + 1);
    if (message->data == NULL) {
        free(message);
        return NULL;
    }
    int current_idx = 0;
    message->data[current_idx++] = SOCKS_VERSION;
    message->data[current_idx++] = choice;
    return message;
}

char parse_server_choice(const message_t *message, bool allow_print_error) {
    if (message == NULL) {
        return -1;
    }
    if (message->data == NULL) {
        return -1;
    }
    if (message->len < 1 + 1) {
        if (allow_print_error) fprintf(stderr, "=== Bad length: %zu / 2\n", message->len);
        return -1;
    }
    char socks_version = message->data[0];
    if (socks_version != SOCKS_VERSION) {
        if (allow_print_error) fprintf(stderr, "=== Bad socks version: %d\n", (int) socks_version);
        return -1;
    };
    return message->data[1];
}

client_greeting_t *parse_client_greeting(const message_t *message, bool allow_print_error) {
    if (message == NULL) {
        return NULL;
    }
    if (message->data == NULL) {
        return NULL;
    }
    if (message->len < 1 + 1 + 1) {
        if (allow_print_error) fprintf(stderr, "=== Bad length: %zu / 3\n", message->len);
        return NULL;
    }
    int current_idx = 0;
    char socks_version = message->data[current_idx++];
    if (socks_version != SOCKS_VERSION) {
        if (allow_print_error) fprintf(stderr, "=== Bad socks version: %d\n", (int) socks_version);
        return NULL;
    }
    char auths_count = message->data[current_idx++];
    if (auths_count > MAX_AUTHS_COUNT) {
        if (allow_print_error) fprintf(stderr, "=== Bad auths count: %d\n", (int) auths_count);
        return NULL;
    }
    if (message->len < 1 + 1 + auths_count) {
        if (allow_print_error) fprintf(stderr, "=== Bad length: %zu / %d\n", message->len, 2 + auths_count);
        return NULL;
    }
    client_greeting_t *res = (client_greeting_t *) malloc(sizeof(*res));
    if (res == NULL) {
        return NULL;
    }
    res->auths_count = auths_count;
    for (int i = 0; i < auths_count; i++) {
        res->auths[i] = message->data[current_idx++];
    }
    return res;
}

/*
 * 	           VER CMD RSV  DSTADDR	  DSTPORT
    Byte Count	1	1	1  Variable	     2
 */
message_t *create_conn_request_message(const conn_request_info_t *info) {
    assert(info);
    struct socks_packet_t packet = {
            .port = info->dest_port,
            .address_type = info->address_type,
            .code = info->command_code
    };
    strcpy(packet.address, info->dest_address);
    return write_packet(packet);
}

/*
 * 	       VER STATUS RSV	BNDADDR	 BNDPORT
Byte Count	1	 1	   1	variable	2
 */
message_t *create_server_response_message(server_response_t *response) {
    assert(response);
    struct socks_packet_t packet = {
            .port = response->bind_port,
            .address_type = response->address_type,
            .code = response->status_code
    };
    strcpy(packet.address, response->bind_address);
    return write_packet(packet);
}

static struct socks_packet_t *parse_packet_message(const message_t *message, bool allow_print_error) {
    assert(message);
    assert(message->data);
    size_t len = message->len;
    if (len <= 1 + 1 + 1 + 2 + 2) {
        if (allow_print_error) fprintf(stderr, "bad length\n");
        return NULL;
    }
    size_t current_idx = 0;
    char socks_version = message->data[current_idx++];
    if (socks_version != SOCKS_VERSION) {
        if (allow_print_error) fprintf(stderr, "=== bad socks version: %d\n", (int) socks_version);
        return NULL;
    }
    char code = message->data[current_idx++];
    current_idx++; // for the reserved zero byte
    char address_type = message->data[current_idx++];
    if (address_type != IPV4_TYPE && address_type != DOMAIN_TYPE) {
        if (allow_print_error) fprintf(stderr, "only ipv4 and domains support\n");
        return NULL;
    }
    char address[ADDR_BUFFER_SIZE];
    if (address_type == IPV4_TYPE) {
        if (current_idx + 4 > len) {
            if (allow_print_error) fprintf(stderr, "not enough length for address\n");
            return NULL;
        }
        const char *binary_addr = &message->data[current_idx];
        sprintf(address, "%hhu.%hhu.%hhu.%hhu", binary_addr[0], binary_addr[1], binary_addr[2], binary_addr[3]);
        current_idx += 4;
    } else { //domain
        char addr_len = message->data[current_idx++];
        for (int i = 0; i < addr_len; i++) {
            address[i] = message->data[current_idx++];
        }
        address[(int) addr_len] = 0;
    }
    if (current_idx + 2 > len) {
        if (allow_print_error) fprintf(stderr, "not enough length for port\n");
        return NULL;
    }
    uint16_t port = (message->data[current_idx + 1] << 8) | message->data[current_idx];
    struct socks_packet_t *res = (struct socks_packet_t *) malloc(sizeof(*res));
    if (NULL == res) {
        if (allow_print_error) fprintf(stderr, "memory error\n");
        return NULL;
    }
    res->code = code;
    strcpy(res->address, address);
    res->port = ntohs(port);
    res->address_type = address_type;
    return res;
}

/*
 * returns NULL in case of error
 */
conn_request_info_t *parse_conn_request_message(const message_t *message, bool allow_print_error) {
    assert(message);
    assert(message->data);
    struct socks_packet_t *packet = parse_packet_message(message, allow_print_error);
    if (NULL == packet) {
        return NULL;
    }
    conn_request_info_t *res = (conn_request_info_t *) malloc(sizeof(*res));
    if (NULL == res) {
        free(packet);
        if (allow_print_error) fprintf(stderr, "memory error\n");
        return NULL;
    }
    res->command_code = packet->code;
    strcpy(res->dest_address, packet->address);
    res->dest_port = packet->port;
    res->address_type = packet->address_type;
    free(packet);
    return res;
}

/*
 * returns NULL in case of error
 */
server_response_t *parse_response_message(const message_t *message, bool allow_print_error) {
    assert(message);
    assert(message->data);
    struct socks_packet_t *packet = parse_packet_message(message, allow_print_error);
    if (NULL == packet) {
        return NULL;
    }
    server_response_t *res = (server_response_t *) malloc(sizeof(*res));
    if (NULL == res) {
        free(packet);
        if (allow_print_error) fprintf(stderr, "memory error\n");
        return NULL;
    }
    res->status_code = packet->code;
    strcpy(res->bind_address, packet->address);
    res->bind_port = packet->port;
    res->address_type = packet->address_type;
    free(packet);
    return res;
}

