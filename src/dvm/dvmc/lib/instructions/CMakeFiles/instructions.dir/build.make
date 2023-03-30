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
CMAKE_SOURCE_DIR = /home/ioc/DIONS/src/dvm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/DIONS/src/dvm

# Include any dependencies generated for this target.
include dvmc/lib/instructions/CMakeFiles/instructions.dir/depend.make

# Include the progress variables for this target.
include dvmc/lib/instructions/CMakeFiles/instructions.dir/progress.make

# Include the compile flags for this target's objects.
include dvmc/lib/instructions/CMakeFiles/instructions.dir/flags.make

dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_metrics.c.o: dvmc/lib/instructions/CMakeFiles/instructions.dir/flags.make
dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_metrics.c.o: dvmc/lib/instructions/instruction_metrics.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_metrics.c.o"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/instructions.dir/instruction_metrics.c.o   -c /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/instruction_metrics.c

dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_metrics.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/instructions.dir/instruction_metrics.c.i"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/instruction_metrics.c > CMakeFiles/instructions.dir/instruction_metrics.c.i

dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_metrics.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/instructions.dir/instruction_metrics.c.s"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/instruction_metrics.c -o CMakeFiles/instructions.dir/instruction_metrics.c.s

dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_names.c.o: dvmc/lib/instructions/CMakeFiles/instructions.dir/flags.make
dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_names.c.o: dvmc/lib/instructions/instruction_names.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building C object dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_names.c.o"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/instructions.dir/instruction_names.c.o   -c /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/instruction_names.c

dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_names.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/instructions.dir/instruction_names.c.i"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/instruction_names.c > CMakeFiles/instructions.dir/instruction_names.c.i

dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_names.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/instructions.dir/instruction_names.c.s"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && /usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/instruction_names.c -o CMakeFiles/instructions.dir/instruction_names.c.s

# Object files for target instructions
instructions_OBJECTS = \
"CMakeFiles/instructions.dir/instruction_metrics.c.o" \
"CMakeFiles/instructions.dir/instruction_names.c.o"

# External object files for target instructions
instructions_EXTERNAL_OBJECTS =

dvmc/lib/instructions/libdvmc-instructions.a: dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_metrics.c.o
dvmc/lib/instructions/libdvmc-instructions.a: dvmc/lib/instructions/CMakeFiles/instructions.dir/instruction_names.c.o
dvmc/lib/instructions/libdvmc-instructions.a: dvmc/lib/instructions/CMakeFiles/instructions.dir/build.make
dvmc/lib/instructions/libdvmc-instructions.a: dvmc/lib/instructions/CMakeFiles/instructions.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/DIONS/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Linking C static library libdvmc-instructions.a"
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && $(CMAKE_COMMAND) -P CMakeFiles/instructions.dir/cmake_clean_target.cmake
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/instructions.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dvmc/lib/instructions/CMakeFiles/instructions.dir/build: dvmc/lib/instructions/libdvmc-instructions.a

.PHONY : dvmc/lib/instructions/CMakeFiles/instructions.dir/build

dvmc/lib/instructions/CMakeFiles/instructions.dir/clean:
	cd /home/ioc/DIONS/src/dvm/dvmc/lib/instructions && $(CMAKE_COMMAND) -P CMakeFiles/instructions.dir/cmake_clean.cmake
.PHONY : dvmc/lib/instructions/CMakeFiles/instructions.dir/clean

dvmc/lib/instructions/CMakeFiles/instructions.dir/depend:
	cd /home/ioc/DIONS/src/dvm && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/DIONS/src/dvm /home/ioc/DIONS/src/dvm/dvmc/lib/instructions /home/ioc/DIONS/src/dvm /home/ioc/DIONS/src/dvm/dvmc/lib/instructions /home/ioc/DIONS/src/dvm/dvmc/lib/instructions/CMakeFiles/instructions.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dvmc/lib/instructions/CMakeFiles/instructions.dir/depend
