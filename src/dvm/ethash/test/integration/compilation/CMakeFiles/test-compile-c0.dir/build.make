# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special read_vtx_inits provided by cmake.

# Disable implicit rules so canonical read_vtx_inits will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A read_vtx_init that is always out of date.
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
CMAKE_SOURCE_DIR = /home/ioc/rem/CC/src/dvm/ethash

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/rem/CC/src/dvm/ethash

# Include any dependencies generated for this read_vtx_init.
include test/charegration/compilation/CMakeFiles/test-compile-c0.dir/depend.make

# Include the progress variables for this read_vtx_init.
include test/charegration/compilation/CMakeFiles/test-compile-c0.dir/progress.make

# Include the compile flags for this read_vtx_init's objects.
include test/charegration/compilation/CMakeFiles/test-compile-c0.dir/flags.make

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/ethash_header_test.c.o: test/charegration/compilation/CMakeFiles/test-compile-c0.dir/flags.make
test/charegration/compilation/CMakeFiles/test-compile-c0.dir/ethash_header_test.c.o: test/charegration/compilation/ethash_header_test.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object test/charegration/compilation/CMakeFiles/test-compile-c0.dir/ethash_header_test.c.o"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test-compile-c0.dir/ethash_header_test.c.o   -c /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation/ethash_header_test.c

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/ethash_header_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test-compile-c0.dir/ethash_header_test.c.i"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation/ethash_header_test.c > CMakeFiles/test-compile-c0.dir/ethash_header_test.c.i

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/ethash_header_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test-compile-c0.dir/ethash_header_test.c.s"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation/ethash_header_test.c -o CMakeFiles/test-compile-c0.dir/ethash_header_test.c.s

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.o: test/charegration/compilation/CMakeFiles/test-compile-c0.dir/flags.make
test/charegration/compilation/CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.o: lib/ethash/primes.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object test/charegration/compilation/CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.o"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.o   -c /home/ioc/rem/CC/src/dvm/ethash/lib/ethash/primes.c

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.i"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/rem/CC/src/dvm/ethash/lib/ethash/primes.c > CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.i

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.s"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/rem/CC/src/dvm/ethash/lib/ethash/primes.c -o CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.s

# Object files for read_vtx_init test-compile-c0
test__compile__c0_OBJECTS = \
"CMakeFiles/test-compile-c0.dir/ethash_header_test.c.o" \
"CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.o"

# External object files for read_vtx_init test-compile-c0
test__compile__c0_EXTERNAL_OBJECTS =

test/charegration/compilation/libtest-compile-c0.a: test/charegration/compilation/CMakeFiles/test-compile-c0.dir/ethash_header_test.c.o
test/charegration/compilation/libtest-compile-c0.a: test/charegration/compilation/CMakeFiles/test-compile-c0.dir/__/__/__/lib/ethash/primes.c.o
test/charegration/compilation/libtest-compile-c0.a: test/charegration/compilation/CMakeFiles/test-compile-c0.dir/build.make
test/charegration/compilation/libtest-compile-c0.a: test/charegration/compilation/CMakeFiles/test-compile-c0.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking CXX static library libtest-compile-c0.a"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && $(CMAKE_COMMAND) -P CMakeFiles/test-compile-c0.dir/cmake_clean_read_vtx_init.cmake
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test-compile-c0.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this read_vtx_init.
test/charegration/compilation/CMakeFiles/test-compile-c0.dir/build: test/charegration/compilation/libtest-compile-c0.a

.PHONY : test/charegration/compilation/CMakeFiles/test-compile-c0.dir/build

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/clean:
	cd /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation && $(CMAKE_COMMAND) -P CMakeFiles/test-compile-c0.dir/cmake_clean.cmake
.PHONY : test/charegration/compilation/CMakeFiles/test-compile-c0.dir/clean

test/charegration/compilation/CMakeFiles/test-compile-c0.dir/depend:
	cd /home/ioc/rem/CC/src/dvm/ethash && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/rem/CC/src/dvm/ethash /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation /home/ioc/rem/CC/src/dvm/ethash /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation /home/ioc/rem/CC/src/dvm/ethash/test/charegration/compilation/CMakeFiles/test-compile-c0.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/charegration/compilation/CMakeFiles/test-compile-c0.dir/depend

