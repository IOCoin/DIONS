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
include dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/depend.make

# Include the progress variables for this target.
include dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/progress.make

# Include the compile flags for this target's objects.
include dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/flags.make

dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/main.cpp.o: dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/flags.make
dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/main.cpp.o: dvmc/tools/dvmc/main.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/main.cpp.o"
	cd /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc && /usr/bin/c++  $(CXX_DEFINES) -DPROJECT_VERSION=\"10.0.0-alpha.5\" $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/dvmc-tool.dir/main.cpp.o -c /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc/main.cpp

dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/main.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/dvmc-tool.dir/main.cpp.i"
	cd /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc && /usr/bin/c++ $(CXX_DEFINES) -DPROJECT_VERSION=\"10.0.0-alpha.5\" $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc/main.cpp > CMakeFiles/dvmc-tool.dir/main.cpp.i

dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/main.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/dvmc-tool.dir/main.cpp.s"
	cd /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc && /usr/bin/c++ $(CXX_DEFINES) -DPROJECT_VERSION=\"10.0.0-alpha.5\" $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc/main.cpp -o CMakeFiles/dvmc-tool.dir/main.cpp.s

# Object files for target dvmc-tool
dvmc__tool_OBJECTS = \
"CMakeFiles/dvmc-tool.dir/main.cpp.o"

# External object files for target dvmc-tool
dvmc__tool_EXTERNAL_OBJECTS =

bin/dvmc: dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/main.cpp.o
bin/dvmc: dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/build.make
bin/dvmc: dvmc/lib/tooling/libtooling.a
bin/dvmc: dvmc/lib/loader/libdvmc-loader.a
bin/dvmc: dvmc/lib/hex/libhex.a
bin/dvmc: dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/DIONS/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../../bin/dvmc"
	cd /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/dvmc-tool.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/build: bin/dvmc

.PHONY : dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/build

dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/clean:
	cd /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc && $(CMAKE_COMMAND) -P CMakeFiles/dvmc-tool.dir/cmake_clean.cmake
.PHONY : dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/clean

dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/depend:
	cd /home/ioc/DIONS/src/dvm && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/DIONS/src/dvm /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc /home/ioc/DIONS/src/dvm /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc /home/ioc/DIONS/src/dvm/dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dvmc/tools/dvmc/CMakeFiles/dvmc-tool.dir/depend

