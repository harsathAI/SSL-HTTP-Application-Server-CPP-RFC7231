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
CMAKE_SOURCE_DIR = /home/samsepi0l/cpp_code/http_fun_serv

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/samsepi0l/cpp_code/http_fun_serv/build

# Include any dependencies generated for this target.
include CMakeFiles/ssl_server.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/ssl_server.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/ssl_server.dir/flags.make

CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.o: CMakeFiles/ssl_server.dir/flags.make
CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.o: ../SSL_selfsigned_internet_domain_http.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/samsepi0l/cpp_code/http_fun_serv/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.o -c /home/samsepi0l/cpp_code/http_fun_serv/SSL_selfsigned_internet_domain_http.cpp

CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/samsepi0l/cpp_code/http_fun_serv/SSL_selfsigned_internet_domain_http.cpp > CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.i

CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/samsepi0l/cpp_code/http_fun_serv/SSL_selfsigned_internet_domain_http.cpp -o CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.s

# Object files for target ssl_server
ssl_server_OBJECTS = \
"CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.o"

# External object files for target ssl_server
ssl_server_EXTERNAL_OBJECTS =

ssl_server: CMakeFiles/ssl_server.dir/SSL_selfsigned_internet_domain_http.cpp.o
ssl_server: CMakeFiles/ssl_server.dir/build.make
ssl_server: CMakeFiles/ssl_server.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/samsepi0l/cpp_code/http_fun_serv/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ssl_server"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ssl_server.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/ssl_server.dir/build: ssl_server

.PHONY : CMakeFiles/ssl_server.dir/build

CMakeFiles/ssl_server.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/ssl_server.dir/cmake_clean.cmake
.PHONY : CMakeFiles/ssl_server.dir/clean

CMakeFiles/ssl_server.dir/depend:
	cd /home/samsepi0l/cpp_code/http_fun_serv/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/samsepi0l/cpp_code/http_fun_serv /home/samsepi0l/cpp_code/http_fun_serv /home/samsepi0l/cpp_code/http_fun_serv/build /home/samsepi0l/cpp_code/http_fun_serv/build /home/samsepi0l/cpp_code/http_fun_serv/build/CMakeFiles/ssl_server.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/ssl_server.dir/depend

