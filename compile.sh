#!/bin/bash
clang -Wall -pedantic -fsanitize=address server.c socket_operations.c io_operations.c -o build/server
echo "Program server compiled successfully"
clang -Wall -pedantic -fsanitize=address client.c socket_operations.c io_operations.c socks_messages.c -o build/client
echo "Program client compiled successfully"
clang -Wall -pedantic -fsanitize=address socks_proxy.c socket_operations.c io_operations.c socks_messages.c -o build/proxy
echo "Program proxy compiled successfully"

