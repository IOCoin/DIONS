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
CMAKE_SOURCE_DIR = /home/ioc/rem/CC/src/dvm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/rem/CC/src/dvm

# Include any dependencies generated for this target.
include dvmc/lib/tooling/CMakeFiles/tooling.dir/depend.make

# Include the progress variables for this target.
include dvmc/lib/tooling/CMakeFiles/tooling.dir/progress.make

# Include the compile flags for this target's objects.
include dvmc/lib/tooling/CMakeFiles/tooling.dir/flags.make

dvmc/lib/tooling/CMakeFiles/tooling.dir/run.cpp.o: dvmc/lib/tooling/CMakeFiles/tooling.dir/flags.make
dvmc/lib/tooling/CMakeFiles/tooling.dir/run.cpp.o: dvmc/lib/tooling/run.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/rem/CC/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object dvmc/lib/tooling/CMakeFiles/tooling.dir/run.cpp.o"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/tooling.dir/run.cpp.o -c /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling/run.cpp

dvmc/lib/tooling/CMakeFiles/tooling.dir/run.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/tooling.dir/run.cpp.i"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling/run.cpp > CMakeFiles/tooling.dir/run.cpp.i

dvmc/lib/tooling/CMakeFiles/tooling.dir/run.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/tooling.dir/run.cpp.s"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling/run.cpp -o CMakeFiles/tooling.dir/run.cpp.s

# Object files for target tooling
tooling_OBJECTS = \
"CMakeFiles/tooling.dir/run.cpp.o"

# External object files for target tooling
tooling_EXTERNAL_OBJECTS =

dvmc/lib/tooling/libtooling.a: dvmc/lib/tooling/CMakeFiles/tooling.dir/run.cpp.o
dvmc/lib/tooling/libtooling.a: dvmc/lib/tooling/CMakeFiles/tooling.dir/build.make
dvmc/lib/tooling/libtooling.a: dvmc/lib/tooling/CMakeFiles/tooling.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/rem/CC/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libtooling.a"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling && $(CMAKE_COMMAND) -P CMakeFiles/tooling.dir/cmake_clean_target.cmake
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/tooling.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dvmc/lib/tooling/CMakeFiles/tooling.dir/build: dvmc/lib/tooling/libtooling.a

.PHONY : dvmc/lib/tooling/CMakeFiles/tooling.dir/build

dvmc/lib/tooling/CMakeFiles/tooling.dir/clean:
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling && $(CMAKE_COMMAND) -P CMakeFiles/tooling.dir/cmake_clean.cmake
.PHONY : dvmc/lib/tooling/CMakeFiles/tooling.dir/clean

dvmc/lib/tooling/CMakeFiles/tooling.dir/depend:
	cd /home/ioc/rem/CC/src/dvm && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/rem/CC/src/dvm /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling /home/ioc/rem/CC/src/dvm /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling /home/ioc/rem/CC/src/dvm/dvmc/lib/tooling/CMakeFiles/tooling.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dvmc/lib/tooling/CMakeFiles/tooling.dir/depend

