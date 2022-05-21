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
CMAKE_SOURCE_DIR = /home/ioc/rem/CC/src/dvm/ethash

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/rem/CC/src/dvm/ethash

# Include any dependencies generated for this target.
include test/fakeminer/CMakeFiles/ethash-fakeminer.dir/depend.make

# Include the progress variables for this target.
include test/fakeminer/CMakeFiles/ethash-fakeminer.dir/progress.make

# Include the compile flags for this target's objects.
include test/fakeminer/CMakeFiles/ethash-fakeminer.dir/flags.make

test/fakeminer/CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.o: test/fakeminer/CMakeFiles/ethash-fakeminer.dir/flags.make
test/fakeminer/CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.o: test/fakeminer/fakeminer.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/fakeminer/CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.o"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.o -c /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer/fakeminer.cpp

test/fakeminer/CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.i"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer/fakeminer.cpp > CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.i

test/fakeminer/CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.s"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer/fakeminer.cpp -o CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.s

# Object files for target ethash-fakeminer
ethash__fakeminer_OBJECTS = \
"CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.o"

# External object files for target ethash-fakeminer
ethash__fakeminer_EXTERNAL_OBJECTS =

bin/ethash-fakeminer: test/fakeminer/CMakeFiles/ethash-fakeminer.dir/fakeminer.cpp.o
bin/ethash-fakeminer: test/fakeminer/CMakeFiles/ethash-fakeminer.dir/build.make
bin/ethash-fakeminer: lib/global_context/libethash-global-context.a
bin/ethash-fakeminer: lib/ethash/libethash.a
bin/ethash-fakeminer: lib/keccak/libkeccak.a
bin/ethash-fakeminer: test/fakeminer/CMakeFiles/ethash-fakeminer.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable ../../bin/ethash-fakeminer"
	cd /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ethash-fakeminer.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/fakeminer/CMakeFiles/ethash-fakeminer.dir/build: bin/ethash-fakeminer

.PHONY : test/fakeminer/CMakeFiles/ethash-fakeminer.dir/build

test/fakeminer/CMakeFiles/ethash-fakeminer.dir/clean:
	cd /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer && $(CMAKE_COMMAND) -P CMakeFiles/ethash-fakeminer.dir/cmake_clean.cmake
.PHONY : test/fakeminer/CMakeFiles/ethash-fakeminer.dir/clean

test/fakeminer/CMakeFiles/ethash-fakeminer.dir/depend:
	cd /home/ioc/rem/CC/src/dvm/ethash && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/rem/CC/src/dvm/ethash /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer /home/ioc/rem/CC/src/dvm/ethash /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer /home/ioc/rem/CC/src/dvm/ethash/test/fakeminer/CMakeFiles/ethash-fakeminer.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/fakeminer/CMakeFiles/ethash-fakeminer.dir/depend

