# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/proxy_server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/proxy_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/proxy_server.dir/flags.make

CMakeFiles/proxy_server.dir/main.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/main.c.o: ../main.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/proxy_server.dir/main.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/main.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/main.c

CMakeFiles/proxy_server.dir/main.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/main.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/main.c > CMakeFiles/proxy_server.dir/main.c.i

CMakeFiles/proxy_server.dir/main.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/main.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/main.c -o CMakeFiles/proxy_server.dir/main.c.s

CMakeFiles/proxy_server.dir/server.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/server.c.o: ../server.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object CMakeFiles/proxy_server.dir/server.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/server.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/server.c

CMakeFiles/proxy_server.dir/server.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/server.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/server.c > CMakeFiles/proxy_server.dir/server.c.i

CMakeFiles/proxy_server.dir/server.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/server.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/server.c -o CMakeFiles/proxy_server.dir/server.c.s

CMakeFiles/proxy_server.dir/client.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/client.c.o: ../client.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building C object CMakeFiles/proxy_server.dir/client.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/client.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/client.c

CMakeFiles/proxy_server.dir/client.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/client.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/client.c > CMakeFiles/proxy_server.dir/client.c.i

CMakeFiles/proxy_server.dir/client.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/client.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/client.c -o CMakeFiles/proxy_server.dir/client.c.s

CMakeFiles/proxy_server.dir/io_operations.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/io_operations.c.o: ../io_operations.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building C object CMakeFiles/proxy_server.dir/io_operations.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/io_operations.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/io_operations.c

CMakeFiles/proxy_server.dir/io_operations.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/io_operations.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/io_operations.c > CMakeFiles/proxy_server.dir/io_operations.c.i

CMakeFiles/proxy_server.dir/io_operations.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/io_operations.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/io_operations.c -o CMakeFiles/proxy_server.dir/io_operations.c.s

CMakeFiles/proxy_server.dir/socks_proxy.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/socks_proxy.c.o: ../socks_proxy.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building C object CMakeFiles/proxy_server.dir/socks_proxy.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/socks_proxy.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socks_proxy.c

CMakeFiles/proxy_server.dir/socks_proxy.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/socks_proxy.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socks_proxy.c > CMakeFiles/proxy_server.dir/socks_proxy.c.i

CMakeFiles/proxy_server.dir/socks_proxy.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/socks_proxy.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socks_proxy.c -o CMakeFiles/proxy_server.dir/socks_proxy.c.s

CMakeFiles/proxy_server.dir/socket_operations.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/socket_operations.c.o: ../socket_operations.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building C object CMakeFiles/proxy_server.dir/socket_operations.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/socket_operations.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socket_operations.c

CMakeFiles/proxy_server.dir/socket_operations.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/socket_operations.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socket_operations.c > CMakeFiles/proxy_server.dir/socket_operations.c.i

CMakeFiles/proxy_server.dir/socket_operations.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/socket_operations.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socket_operations.c -o CMakeFiles/proxy_server.dir/socket_operations.c.s

CMakeFiles/proxy_server.dir/pipe_operations.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/pipe_operations.c.o: ../pipe_operations.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building C object CMakeFiles/proxy_server.dir/pipe_operations.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/pipe_operations.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/pipe_operations.c

CMakeFiles/proxy_server.dir/pipe_operations.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/pipe_operations.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/pipe_operations.c > CMakeFiles/proxy_server.dir/pipe_operations.c.i

CMakeFiles/proxy_server.dir/pipe_operations.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/pipe_operations.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/pipe_operations.c -o CMakeFiles/proxy_server.dir/pipe_operations.c.s

CMakeFiles/proxy_server.dir/socks_messages.c.o: CMakeFiles/proxy_server.dir/flags.make
CMakeFiles/proxy_server.dir/socks_messages.c.o: ../socks_messages.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building C object CMakeFiles/proxy_server.dir/socks_messages.c.o"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/proxy_server.dir/socks_messages.c.o   -c /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socks_messages.c

CMakeFiles/proxy_server.dir/socks_messages.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/proxy_server.dir/socks_messages.c.i"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socks_messages.c > CMakeFiles/proxy_server.dir/socks_messages.c.i

CMakeFiles/proxy_server.dir/socks_messages.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/proxy_server.dir/socks_messages.c.s"
	/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/socks_messages.c -o CMakeFiles/proxy_server.dir/socks_messages.c.s

# Object files for target proxy_server
proxy_server_OBJECTS = \
"CMakeFiles/proxy_server.dir/main.c.o" \
"CMakeFiles/proxy_server.dir/server.c.o" \
"CMakeFiles/proxy_server.dir/client.c.o" \
"CMakeFiles/proxy_server.dir/io_operations.c.o" \
"CMakeFiles/proxy_server.dir/socks_proxy.c.o" \
"CMakeFiles/proxy_server.dir/socket_operations.c.o" \
"CMakeFiles/proxy_server.dir/pipe_operations.c.o" \
"CMakeFiles/proxy_server.dir/socks_messages.c.o"

# External object files for target proxy_server
proxy_server_EXTERNAL_OBJECTS =

proxy_server: CMakeFiles/proxy_server.dir/main.c.o
proxy_server: CMakeFiles/proxy_server.dir/server.c.o
proxy_server: CMakeFiles/proxy_server.dir/client.c.o
proxy_server: CMakeFiles/proxy_server.dir/io_operations.c.o
proxy_server: CMakeFiles/proxy_server.dir/socks_proxy.c.o
proxy_server: CMakeFiles/proxy_server.dir/socket_operations.c.o
proxy_server: CMakeFiles/proxy_server.dir/pipe_operations.c.o
proxy_server: CMakeFiles/proxy_server.dir/socks_messages.c.o
proxy_server: CMakeFiles/proxy_server.dir/build.make
proxy_server: CMakeFiles/proxy_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Linking C executable proxy_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/proxy_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/proxy_server.dir/build: proxy_server

.PHONY : CMakeFiles/proxy_server.dir/build

CMakeFiles/proxy_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/proxy_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/proxy_server.dir/clean

CMakeFiles/proxy_server.dir/depend:
	cd /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug /mnt/c/Users/ms_dr/CLionProjects/networks/socks_proxy/cmake-build-debug/CMakeFiles/proxy_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/proxy_server.dir/depend

