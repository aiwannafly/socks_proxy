cmake_minimum_required(VERSION 3.16)
project(proxy_server C)

set(CMAKE_C_STANDARD 99)

add_executable(proxy_server main.c server.c client.c io_operations.h io_operations.c
        socks_proxy.c socket_operations.c socket_operations.h pipe_operations.h pipe_operations.c socks_messages.c socks_messages.h)
