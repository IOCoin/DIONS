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
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ioc/DIONS/src/dvm/ethash

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/DIONS/src/dvm/ethash

# Include any dependencies generated for this target.
include lib/ethash/CMakeFiles/ethash.dir/depend.make

# Include the progress variables for this target.
include lib/ethash/CMakeFiles/ethash.dir/progress.make

# Include the compile flags for this target's objects.
include lib/ethash/CMakeFiles/ethash.dir/flags.make

lib/ethash/CMakeFiles/ethash.dir/ethash.cpp.o: lib/ethash/CMakeFiles/ethash.dir/flags.make
lib/ethash/CMakeFiles/ethash.dir/ethash.cpp.o: lib/ethash/ethash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/ethash/CMakeFiles/ethash.dir/ethash.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash.dir/ethash.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/lib/ethash/ethash.cpp

lib/ethash/CMakeFiles/ethash.dir/ethash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash.dir/ethash.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/lib/ethash/ethash.cpp > CMakeFiles/ethash.dir/ethash.cpp.i

lib/ethash/CMakeFiles/ethash.dir/ethash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash.dir/ethash.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/lib/ethash/ethash.cpp -o CMakeFiles/ethash.dir/ethash.cpp.s

lib/ethash/CMakeFiles/ethash.dir/primes.c.o: lib/ethash/CMakeFiles/ethash.dir/flags.make
lib/ethash/CMakeFiles/ethash.dir/primes.c.o: lib/ethash/primes.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object lib/ethash/CMakeFiles/ethash.dir/primes.c.o"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/ethash.dir/primes.c.o   -c /home/ioc/DIONS/src/dvm/ethash/lib/ethash/primes.c

lib/ethash/CMakeFiles/ethash.dir/primes.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/ethash.dir/primes.c.i"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/lib/ethash/primes.c > CMakeFiles/ethash.dir/primes.c.i

lib/ethash/CMakeFiles/ethash.dir/primes.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/ethash.dir/primes.c.s"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/lib/ethash/primes.c -o CMakeFiles/ethash.dir/primes.c.s

# Object files for target ethash
ethash_OBJECTS = \
"CMakeFiles/ethash.dir/ethash.cpp.o" \
"CMakeFiles/ethash.dir/primes.c.o"

# External object files for target ethash
ethash_EXTERNAL_OBJECTS =

lib/ethash/libethash.a: lib/ethash/CMakeFiles/ethash.dir/ethash.cpp.o
lib/ethash/libethash.a: lib/ethash/CMakeFiles/ethash.dir/primes.c.o
lib/ethash/libethash.a: lib/ethash/CMakeFiles/ethash.dir/build.make
lib/ethash/libethash.a: lib/ethash/CMakeFiles/ethash.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libethash.a"
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && $(CMAKE_COMMAND) -P CMakeFiles/ethash.dir/cmake_clean_target.cmake
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ethash.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/ethash/CMakeFiles/ethash.dir/build: lib/ethash/libethash.a

.PHONY : lib/ethash/CMakeFiles/ethash.dir/build

lib/ethash/CMakeFiles/ethash.dir/clean:
	cd /home/ioc/DIONS/src/dvm/ethash/lib/ethash && $(CMAKE_COMMAND) -P CMakeFiles/ethash.dir/cmake_clean.cmake
.PHONY : lib/ethash/CMakeFiles/ethash.dir/clean

lib/ethash/CMakeFiles/ethash.dir/depend:
	cd /home/ioc/DIONS/src/dvm/ethash && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/DIONS/src/dvm/ethash /home/ioc/DIONS/src/dvm/ethash/lib/ethash /home/ioc/DIONS/src/dvm/ethash /home/ioc/DIONS/src/dvm/ethash/lib/ethash /home/ioc/DIONS/src/dvm/ethash/lib/ethash/CMakeFiles/ethash.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/ethash/CMakeFiles/ethash.dir/depend

