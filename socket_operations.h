#ifndef PROXY_SERVER_SOCKET_OPERATIONS_H
#define PROXY_SERVER_SOCKET_OPERATIONS_H

#include <arpa/inet.h>

int set_nonblocking(int serv_socket);

int set_reusable(int serv_socket);

int connect_to_address(char *serv_ipv4_address, int port,
                       struct timeval *timeout);

int make_new_connection_sockaddr(struct sockaddr_in *addr, int port);

#endif //PROXY_SERVER_SOCKET_OPERATIONS_H
