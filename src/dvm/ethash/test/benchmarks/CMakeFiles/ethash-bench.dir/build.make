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
include test/benchmarks/CMakeFiles/ethash-bench.dir/depend.make

# Include the progress variables for this target.
include test/benchmarks/CMakeFiles/ethash-bench.dir/progress.make

# Include the compile flags for this target's objects.
include test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make

test/benchmarks/CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.o: test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make
test/benchmarks/CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.o: test/benchmarks/ethash_benchmarks.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/benchmarks/CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/ethash_benchmarks.cpp

test/benchmarks/CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/ethash_benchmarks.cpp > CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.i

test/benchmarks/CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/ethash_benchmarks.cpp -o CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.s

test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.o: test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make
test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.o: test/benchmarks/keccak_benchmarks.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/keccak_benchmarks.cpp

test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/keccak_benchmarks.cpp > CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.i

test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/keccak_benchmarks.cpp -o CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.s

test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_utils.cpp.o: test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make
test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_utils.cpp.o: test/benchmarks/keccak_utils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_utils.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-bench.dir/keccak_utils.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/keccak_utils.cpp

test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-bench.dir/keccak_utils.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/keccak_utils.cpp > CMakeFiles/ethash-bench.dir/keccak_utils.cpp.i

test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-bench.dir/keccak_utils.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/keccak_utils.cpp -o CMakeFiles/ethash-bench.dir/keccak_utils.cpp.s

test/benchmarks/CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.o: test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make
test/benchmarks/CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.o: test/benchmarks/global_context_benchmarks.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object test/benchmarks/CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/global_context_benchmarks.cpp

test/benchmarks/CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/global_context_benchmarks.cpp > CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.i

test/benchmarks/CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/global_context_benchmarks.cpp -o CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.s

test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.o: test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make
test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.o: test/benchmarks/threadsync_benchmarks.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/threadsync_benchmarks.cpp

test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/threadsync_benchmarks.cpp > CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.i

test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/threadsync_benchmarks.cpp -o CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.s

test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.o: test/benchmarks/CMakeFiles/ethash-bench.dir/flags.make
test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.o: test/benchmarks/threadsync_utils.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.o"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.o -c /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/threadsync_utils.cpp

test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.i"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/threadsync_utils.cpp > CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.i

test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.s"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/threadsync_utils.cpp -o CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.s

# Object files for target ethash-bench
ethash__bench_OBJECTS = \
"CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.o" \
"CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.o" \
"CMakeFiles/ethash-bench.dir/keccak_utils.cpp.o" \
"CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.o" \
"CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.o" \
"CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.o"

# External object files for target ethash-bench
ethash__bench_EXTERNAL_OBJECTS =

test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/ethash_benchmarks.cpp.o
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_benchmarks.cpp.o
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/keccak_utils.cpp.o
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/global_context_benchmarks.cpp.o
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_benchmarks.cpp.o
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/threadsync_utils.cpp.o
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/build.make
test/ethash-bench: lib/global_context/libethash-global-context.a
test/ethash-bench: /home/ioc/.hunter/_Base/a3d7f43/97671df/104ac99/Install/lib/libbenchmark.a
test/ethash-bench: lib/ethash/libethash.a
test/ethash-bench: lib/keccak/libkeccak.a
test/ethash-bench: /usr/lib/x86_64-linux-gnu/librt.so
test/ethash-bench: test/benchmarks/CMakeFiles/ethash-bench.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/DIONS/src/dvm/ethash/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Linking CXX executable ../ethash-bench"
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/ethash-bench.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/benchmarks/CMakeFiles/ethash-bench.dir/build: test/ethash-bench

.PHONY : test/benchmarks/CMakeFiles/ethash-bench.dir/build

test/benchmarks/CMakeFiles/ethash-bench.dir/clean:
	cd /home/ioc/DIONS/src/dvm/ethash/test/benchmarks && $(CMAKE_COMMAND) -P CMakeFiles/ethash-bench.dir/cmake_clean.cmake
.PHONY : test/benchmarks/CMakeFiles/ethash-bench.dir/clean

test/benchmarks/CMakeFiles/ethash-bench.dir/depend:
	cd /home/ioc/DIONS/src/dvm/ethash && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/DIONS/src/dvm/ethash /home/ioc/DIONS/src/dvm/ethash/test/benchmarks /home/ioc/DIONS/src/dvm/ethash /home/ioc/DIONS/src/dvm/ethash/test/benchmarks /home/ioc/DIONS/src/dvm/ethash/test/benchmarks/CMakeFiles/ethash-bench.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/benchmarks/CMakeFiles/ethash-bench.dir/depend

