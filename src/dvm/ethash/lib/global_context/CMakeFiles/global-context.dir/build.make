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
include lib/global_context/CMakeFiles/global-context.dir/depend.make

# Include the progress variables for this target.
include lib/global_context/CMakeFiles/global-context.dir/progress.make

# Include the compile flags for this target's objects.
include lib/global_context/CMakeFiles/global-context.dir/flags.make

lib/global_context/CMakeFiles/global-context.dir/global_context.cpp.o: lib/global_context/CMakeFiles/global-context.dir/flags.make
lib/global_context/CMakeFiles/global-context.dir/global_context.cpp.o: lib/global_context/global_context.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object lib/global_context/CMakeFiles/global-context.dir/global_context.cpp.o"
	cd /home/ioc/rem/CC/src/dvm/ethash/lib/global_context && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/global-context.dir/global_context.cpp.o -c /home/ioc/rem/CC/src/dvm/ethash/lib/global_context/global_context.cpp

lib/global_context/CMakeFiles/global-context.dir/global_context.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/global-context.dir/global_context.cpp.i"
	cd /home/ioc/rem/CC/src/dvm/ethash/lib/global_context && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/rem/CC/src/dvm/ethash/lib/global_context/global_context.cpp > CMakeFiles/global-context.dir/global_context.cpp.i

lib/global_context/CMakeFiles/global-context.dir/global_context.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/global-context.dir/global_context.cpp.s"
	cd /home/ioc/rem/CC/src/dvm/ethash/lib/global_context && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/rem/CC/src/dvm/ethash/lib/global_context/global_context.cpp -o CMakeFiles/global-context.dir/global_context.cpp.s

# Object files for target global-context
global__context_OBJECTS = \
"CMakeFiles/global-context.dir/global_context.cpp.o"

# External object files for target global-context
global__context_EXTERNAL_OBJECTS =

lib/global_context/libethash-global-context.a: lib/global_context/CMakeFiles/global-context.dir/global_context.cpp.o
lib/global_context/libethash-global-context.a: lib/global_context/CMakeFiles/global-context.dir/build.make
lib/global_context/libethash-global-context.a: lib/global_context/CMakeFiles/global-context.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/rem/CC/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX static library libethash-global-context.a"
	cd /home/ioc/rem/CC/src/dvm/ethash/lib/global_context && $(CMAKE_COMMAND) -P CMakeFiles/global-context.dir/cmake_clean_target.cmake
	cd /home/ioc/rem/CC/src/dvm/ethash/lib/global_context && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/global-context.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
lib/global_context/CMakeFiles/global-context.dir/build: lib/global_context/libethash-global-context.a

.PHONY : lib/global_context/CMakeFiles/global-context.dir/build

lib/global_context/CMakeFiles/global-context.dir/clean:
	cd /home/ioc/rem/CC/src/dvm/ethash/lib/global_context && $(CMAKE_COMMAND) -P CMakeFiles/global-context.dir/cmake_clean.cmake
.PHONY : lib/global_context/CMakeFiles/global-context.dir/clean

lib/global_context/CMakeFiles/global-context.dir/depend:
	cd /home/ioc/rem/CC/src/dvm/ethash && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/rem/CC/src/dvm/ethash /home/ioc/rem/CC/src/dvm/ethash/lib/global_context /home/ioc/rem/CC/src/dvm/ethash /home/ioc/rem/CC/src/dvm/ethash/lib/global_context /home/ioc/rem/CC/src/dvm/ethash/lib/global_context/CMakeFiles/global-context.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : lib/global_context/CMakeFiles/global-context.dir/depend

