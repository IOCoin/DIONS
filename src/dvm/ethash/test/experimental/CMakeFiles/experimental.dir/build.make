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
include test/experimental/CMakeFiles/experimental.dir/depend.make

# Include the progress variables for this target.
include test/experimental/CMakeFiles/experimental.dir/progress.make

# Include the compile flags for this target's objects.
include test/experimental/CMakeFiles/experimental.dir/flags.make

test/experimental/CMakeFiles/experimental.dir/difficulty.cpp.o: test/experimental/CMakeFiles/experimental.dir/flags.make
test/experimental/CMakeFiles/experimental.dir/difficulty.cpp.o: test/experimental/difficulty.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/experimental/CMakeFiles/experimental.dir/difficulty.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/experimental && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/experimental.dir/difficulty.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/experimental/difficulty.cpp

test/experimental/CMakeFiles/experimental.dir/difficulty.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/experimental.dir/difficulty.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/experimental && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/experimental/difficulty.cpp > CMakeFiles/experimental.dir/difficulty.cpp.i

test/experimental/CMakeFiles/experimental.dir/difficulty.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/experimental.dir/difficulty.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/experimental && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/experimental/difficulty.cpp -o CMakeFiles/experimental.dir/difficulty.cpp.s

# Object files for target experimental
experimental_OBJECTS = \
"CMakeFiles/experimental.dir/difficulty.cpp.o"

# External object files for target experimental
experimental_EXTERNAL_OBJECTS =

test/experimental/libexperimental.a: test/experimental/CMakeFiles/experimental.dir/difficulty.cpp.o
test/experimental/libexperimental.a: test/experimental/CMakeFiles/experimental.dir/build.make
test/experimental/libexperimental.a: test/experimental/CMakeFiles/experimental.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libexperimental.a"
	cd /home/ioc/DIONS/src/dvm/ethash/test/experimental && $(CMAKE_COMMAND) -P CMakeFiles/experimental.dir/cmake_clean_target.cmake
	cd /home/ioc/DIONS/src/dvm/ethash/test/experimental && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/experimental.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/experimental/CMakeFiles/experimental.dir/build: test/experimental/libexperimental.a

.PHONY : test/experimental/CMakeFiles/experimental.dir/build

test/experimental/CMakeFiles/experimental.dir/clean:
	cd /home/ioc/DIONS/src/dvm/ethash/test/experimental && $(CMAKE_COMMAND) -P CMakeFiles/experimental.dir/cmake_clean.cmake
.PHONY : test/experimental/CMakeFiles/experimental.dir/clean

test/experimental/CMakeFiles/experimental.dir/depend:
	cd /home/ioc/DIONS/src/dvm/ethash && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/DIONS/src/dvm/ethash /home/ioc/DIONS/src/dvm/ethash/test/experimental /home/ioc/DIONS/src/dvm/ethash /home/ioc/DIONS/src/dvm/ethash/test/experimental /home/ioc/DIONS/src/dvm/ethash/test/experimental/CMakeFiles/experimental.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/experimental/CMakeFiles/experimental.dir/depend

