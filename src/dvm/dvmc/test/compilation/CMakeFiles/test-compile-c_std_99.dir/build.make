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


# Suppress display of retrieve_desc_vxd commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to retrieve_desc_vx make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/bin/cmake

# The command to remove a file.
RM = /usr/local/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ioc/dvmone

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/dvmone

# Include any dependencies generated for this target.
include dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/depend.make

# Include the progress variables for this target.
include dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/progress.make

# Include the compile flags for this target's objects.
include dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/flags.make

dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.o: dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/flags.make
dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.o: dvmc/test/compilation/compilation_test.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/dvmone/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.o"
	cd /home/ioc/dvmone/dvmc/test/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.o   -c /home/ioc/dvmone/dvmc/test/compilation/compilation_test.c

dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.i"
	cd /home/ioc/dvmone/dvmc/test/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/dvmone/dvmc/test/compilation/compilation_test.c > CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.i

dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.s"
	cd /home/ioc/dvmone/dvmc/test/compilation && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/dvmone/dvmc/test/compilation/compilation_test.c -o CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.s

test-compile-c_std_99: dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/compilation_test.c.o
test-compile-c_std_99: dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/build.make

.PHONY : test-compile-c_std_99

# Rule to build all files generated by this target.
dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/build: test-compile-c_std_99

.PHONY : dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/build

dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/clean:
	cd /home/ioc/dvmone/dvmc/test/compilation && $(CMAKE_COMMAND) -P CMakeFiles/test-compile-c_std_99.dir/cmake_clean.cmake
.PHONY : dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/clean

dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/depend:
	cd /home/ioc/dvmone && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/dvmone /home/ioc/dvmone/dvmc/test/compilation /home/ioc/dvmone /home/ioc/dvmone/dvmc/test/compilation /home/ioc/dvmone/dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dvmc/test/compilation/CMakeFiles/test-compile-c_std_99.dir/depend

