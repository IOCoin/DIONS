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
include dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/depend.make

# Include the progress variables for this target.
include dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/progress.make

# Include the compile flags for this target's objects.
include dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/flags.make

dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.o: dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/flags.make
dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.o: dvmc/trans_logs/trans_log.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/dvmone/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.o"
	cd /home/ioc/dvmone/dvmc/trans_logs && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.o   -c /home/ioc/dvmone/dvmc/trans_logs/trans_log.c

dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.i"
	cd /home/ioc/dvmone/dvmc/trans_logs && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/dvmone/dvmc/trans_logs/trans_log.c > CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.i

dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.s"
	cd /home/ioc/dvmone/dvmc/trans_logs && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/dvmone/dvmc/trans_logs/trans_log.c -o CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.s

# Object files for target dvmc-trans_log-static
dvmc__trans_log__static_OBJECTS = \
"CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.o"

# External object files for target dvmc-trans_log-static
dvmc__trans_log__static_EXTERNAL_OBJECTS =

bin/dvmc-trans_log-static: dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/trans_log.c.o
bin/dvmc-trans_log-static: dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/build.make
bin/dvmc-trans_log-static: dvmc/trans_logs/libdvmc-trans_log-host.a
bin/dvmc-trans_log-static: dvmc/trans_logs/trans_log_vm/libtrans_log-vm-static.a
bin/dvmc-trans_log-static: dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/dvmone/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/dvmc-trans_log-static"
	cd /home/ioc/dvmone/dvmc/trans_logs && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dvmc-trans_log-static.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/build: bin/dvmc-trans_log-static

.PHONY : dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/build

dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/clean:
	cd /home/ioc/dvmone/dvmc/trans_logs && $(CMAKE_COMMAND) -P CMakeFiles/dvmc-trans_log-static.dir/cmake_clean.cmake
.PHONY : dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/clean

dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/depend:
	cd /home/ioc/dvmone && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/dvmone /home/ioc/dvmone/dvmc/trans_logs /home/ioc/dvmone /home/ioc/dvmone/dvmc/trans_logs /home/ioc/dvmone/dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dvmc/trans_logs/CMakeFiles/dvmc-trans_log-static.dir/depend

