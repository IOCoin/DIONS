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
CMAKE_SOURCE_DIR = /home/ioc/rem/CC/src/dvm

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/rem/CC/src/dvm

# Include any dependencies generated for this target.
include dvmc/lib/hex/CMakeFiles/hex.dir/depend.make

# Include the progress variables for this target.
include dvmc/lib/hex/CMakeFiles/hex.dir/progress.make

# Include the compile flags for this target's objects.
include dvmc/lib/hex/CMakeFiles/hex.dir/flags.make

dvmc/lib/hex/CMakeFiles/hex.dir/hex.cpp.o: dvmc/lib/hex/CMakeFiles/hex.dir/flags.make
dvmc/lib/hex/CMakeFiles/hex.dir/hex.cpp.o: dvmc/lib/hex/hex.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/rem/CC/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object dvmc/lib/hex/CMakeFiles/hex.dir/hex.cpp.o"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/hex && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/hex.dir/hex.cpp.o -c /home/ioc/rem/CC/src/dvm/dvmc/lib/hex/hex.cpp

dvmc/lib/hex/CMakeFiles/hex.dir/hex.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/hex.dir/hex.cpp.i"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/hex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/rem/CC/src/dvm/dvmc/lib/hex/hex.cpp > CMakeFiles/hex.dir/hex.cpp.i

dvmc/lib/hex/CMakeFiles/hex.dir/hex.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/hex.dir/hex.cpp.s"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/hex && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/rem/CC/src/dvm/dvmc/lib/hex/hex.cpp -o CMakeFiles/hex.dir/hex.cpp.s

# Object files for target hex
hex_OBJECTS = \
"CMakeFiles/hex.dir/hex.cpp.o"

# External object files for target hex
hex_EXTERNAL_OBJECTS =

dvmc/lib/hex/libhex.a: dvmc/lib/hex/CMakeFiles/hex.dir/hex.cpp.o
dvmc/lib/hex/libhex.a: dvmc/lib/hex/CMakeFiles/hex.dir/build.make
dvmc/lib/hex/libhex.a: dvmc/lib/hex/CMakeFiles/hex.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/rem/CC/src/dvm/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libhex.a"
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/hex && $(CMAKE_COMMAND) -P CMakeFiles/hex.dir/cmake_clean_target.cmake
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/hex && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/hex.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
dvmc/lib/hex/CMakeFiles/hex.dir/build: dvmc/lib/hex/libhex.a

.PHONY : dvmc/lib/hex/CMakeFiles/hex.dir/build

dvmc/lib/hex/CMakeFiles/hex.dir/clean:
	cd /home/ioc/rem/CC/src/dvm/dvmc/lib/hex && $(CMAKE_COMMAND) -P CMakeFiles/hex.dir/cmake_clean.cmake
.PHONY : dvmc/lib/hex/CMakeFiles/hex.dir/clean

dvmc/lib/hex/CMakeFiles/hex.dir/depend:
	cd /home/ioc/rem/CC/src/dvm && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/rem/CC/src/dvm /home/ioc/rem/CC/src/dvm/dvmc/lib/hex /home/ioc/rem/CC/src/dvm /home/ioc/rem/CC/src/dvm/dvmc/lib/hex /home/ioc/rem/CC/src/dvm/dvmc/lib/hex/CMakeFiles/hex.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : dvmc/lib/hex/CMakeFiles/hex.dir/depend

