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
CMAKE_SOURCE_DIR = /home/ioc/DIONS/src/ptrie

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ioc/DIONS/src/ptrie

# Include any dependencies generated for this target.
include CMakeFiles/devcore.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/devcore.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/devcore.dir/flags.make

CMakeFiles/devcore.dir/Address.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/Address.o: Address.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/devcore.dir/Address.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/Address.o -c /home/ioc/DIONS/src/ptrie/Address.cpp

CMakeFiles/devcore.dir/Address.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/Address.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/Address.cpp > CMakeFiles/devcore.dir/Address.i

CMakeFiles/devcore.dir/Address.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/Address.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/Address.cpp -o CMakeFiles/devcore.dir/Address.s

CMakeFiles/devcore.dir/Common.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/Common.o: Common.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Building CXX object CMakeFiles/devcore.dir/Common.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/Common.o -c /home/ioc/DIONS/src/ptrie/Common.cpp

CMakeFiles/devcore.dir/Common.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/Common.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/Common.cpp > CMakeFiles/devcore.dir/Common.i

CMakeFiles/devcore.dir/Common.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/Common.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/Common.cpp -o CMakeFiles/devcore.dir/Common.s

CMakeFiles/devcore.dir/CommonData.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/CommonData.o: CommonData.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_3) "Building CXX object CMakeFiles/devcore.dir/CommonData.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/CommonData.o -c /home/ioc/DIONS/src/ptrie/CommonData.cpp

CMakeFiles/devcore.dir/CommonData.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/CommonData.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/CommonData.cpp > CMakeFiles/devcore.dir/CommonData.i

CMakeFiles/devcore.dir/CommonData.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/CommonData.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/CommonData.cpp -o CMakeFiles/devcore.dir/CommonData.s

CMakeFiles/devcore.dir/DBFactory.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/DBFactory.o: DBFactory.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_4) "Building CXX object CMakeFiles/devcore.dir/DBFactory.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/DBFactory.o -c /home/ioc/DIONS/src/ptrie/DBFactory.cpp

CMakeFiles/devcore.dir/DBFactory.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/DBFactory.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/DBFactory.cpp > CMakeFiles/devcore.dir/DBFactory.i

CMakeFiles/devcore.dir/DBFactory.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/DBFactory.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/DBFactory.cpp -o CMakeFiles/devcore.dir/DBFactory.s

CMakeFiles/devcore.dir/FileSystem.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/FileSystem.o: FileSystem.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_5) "Building CXX object CMakeFiles/devcore.dir/FileSystem.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/FileSystem.o -c /home/ioc/DIONS/src/ptrie/FileSystem.cpp

CMakeFiles/devcore.dir/FileSystem.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/FileSystem.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/FileSystem.cpp > CMakeFiles/devcore.dir/FileSystem.i

CMakeFiles/devcore.dir/FileSystem.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/FileSystem.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/FileSystem.cpp -o CMakeFiles/devcore.dir/FileSystem.s

CMakeFiles/devcore.dir/FixedHash.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/FixedHash.o: FixedHash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_6) "Building CXX object CMakeFiles/devcore.dir/FixedHash.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/FixedHash.o -c /home/ioc/DIONS/src/ptrie/FixedHash.cpp

CMakeFiles/devcore.dir/FixedHash.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/FixedHash.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/FixedHash.cpp > CMakeFiles/devcore.dir/FixedHash.i

CMakeFiles/devcore.dir/FixedHash.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/FixedHash.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/FixedHash.cpp -o CMakeFiles/devcore.dir/FixedHash.s

CMakeFiles/devcore.dir/LevelDB.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/LevelDB.o: LevelDB.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_7) "Building CXX object CMakeFiles/devcore.dir/LevelDB.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/LevelDB.o -c /home/ioc/DIONS/src/ptrie/LevelDB.cpp

CMakeFiles/devcore.dir/LevelDB.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/LevelDB.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/LevelDB.cpp > CMakeFiles/devcore.dir/LevelDB.i

CMakeFiles/devcore.dir/LevelDB.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/LevelDB.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/LevelDB.cpp -o CMakeFiles/devcore.dir/LevelDB.s

CMakeFiles/devcore.dir/OverlayDB.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/OverlayDB.o: OverlayDB.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_8) "Building CXX object CMakeFiles/devcore.dir/OverlayDB.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/OverlayDB.o -c /home/ioc/DIONS/src/ptrie/OverlayDB.cpp

CMakeFiles/devcore.dir/OverlayDB.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/OverlayDB.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/OverlayDB.cpp > CMakeFiles/devcore.dir/OverlayDB.i

CMakeFiles/devcore.dir/OverlayDB.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/OverlayDB.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/OverlayDB.cpp -o CMakeFiles/devcore.dir/OverlayDB.s

CMakeFiles/devcore.dir/RLP.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/RLP.o: RLP.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_9) "Building CXX object CMakeFiles/devcore.dir/RLP.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/RLP.o -c /home/ioc/DIONS/src/ptrie/RLP.cpp

CMakeFiles/devcore.dir/RLP.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/RLP.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/RLP.cpp > CMakeFiles/devcore.dir/RLP.i

CMakeFiles/devcore.dir/RLP.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/RLP.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/RLP.cpp -o CMakeFiles/devcore.dir/RLP.s

CMakeFiles/devcore.dir/SHA3.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/SHA3.o: SHA3.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_10) "Building CXX object CMakeFiles/devcore.dir/SHA3.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/SHA3.o -c /home/ioc/DIONS/src/ptrie/SHA3.cpp

CMakeFiles/devcore.dir/SHA3.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/SHA3.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/SHA3.cpp > CMakeFiles/devcore.dir/SHA3.i

CMakeFiles/devcore.dir/SHA3.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/SHA3.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/SHA3.cpp -o CMakeFiles/devcore.dir/SHA3.s

CMakeFiles/devcore.dir/StateCacheDB.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/StateCacheDB.o: StateCacheDB.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_11) "Building CXX object CMakeFiles/devcore.dir/StateCacheDB.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/StateCacheDB.o -c /home/ioc/DIONS/src/ptrie/StateCacheDB.cpp

CMakeFiles/devcore.dir/StateCacheDB.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/StateCacheDB.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/StateCacheDB.cpp > CMakeFiles/devcore.dir/StateCacheDB.i

CMakeFiles/devcore.dir/StateCacheDB.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/StateCacheDB.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/StateCacheDB.cpp -o CMakeFiles/devcore.dir/StateCacheDB.s

CMakeFiles/devcore.dir/TrieCommon.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/TrieCommon.o: TrieCommon.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_12) "Building CXX object CMakeFiles/devcore.dir/TrieCommon.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/TrieCommon.o -c /home/ioc/DIONS/src/ptrie/TrieCommon.cpp

CMakeFiles/devcore.dir/TrieCommon.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/TrieCommon.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/TrieCommon.cpp > CMakeFiles/devcore.dir/TrieCommon.i

CMakeFiles/devcore.dir/TrieCommon.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/TrieCommon.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/TrieCommon.cpp -o CMakeFiles/devcore.dir/TrieCommon.s

CMakeFiles/devcore.dir/TrieHash.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/TrieHash.o: TrieHash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_13) "Building CXX object CMakeFiles/devcore.dir/TrieHash.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/TrieHash.o -c /home/ioc/DIONS/src/ptrie/TrieHash.cpp

CMakeFiles/devcore.dir/TrieHash.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/TrieHash.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/TrieHash.cpp > CMakeFiles/devcore.dir/TrieHash.i

CMakeFiles/devcore.dir/TrieHash.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/TrieHash.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/TrieHash.cpp -o CMakeFiles/devcore.dir/TrieHash.s

CMakeFiles/devcore.dir/Account.o: CMakeFiles/devcore.dir/flags.make
CMakeFiles/devcore.dir/Account.o: Account.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_14) "Building CXX object CMakeFiles/devcore.dir/Account.o"
	/usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/devcore.dir/Account.o -c /home/ioc/DIONS/src/ptrie/Account.cpp

CMakeFiles/devcore.dir/Account.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/devcore.dir/Account.i"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/ioc/DIONS/src/ptrie/Account.cpp > CMakeFiles/devcore.dir/Account.i

CMakeFiles/devcore.dir/Account.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/devcore.dir/Account.s"
	/usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/ioc/DIONS/src/ptrie/Account.cpp -o CMakeFiles/devcore.dir/Account.s

# Object files for target devcore
devcore_OBJECTS = \
"CMakeFiles/devcore.dir/Address.o" \
"CMakeFiles/devcore.dir/Common.o" \
"CMakeFiles/devcore.dir/CommonData.o" \
"CMakeFiles/devcore.dir/DBFactory.o" \
"CMakeFiles/devcore.dir/FileSystem.o" \
"CMakeFiles/devcore.dir/FixedHash.o" \
"CMakeFiles/devcore.dir/LevelDB.o" \
"CMakeFiles/devcore.dir/OverlayDB.o" \
"CMakeFiles/devcore.dir/RLP.o" \
"CMakeFiles/devcore.dir/SHA3.o" \
"CMakeFiles/devcore.dir/StateCacheDB.o" \
"CMakeFiles/devcore.dir/TrieCommon.o" \
"CMakeFiles/devcore.dir/TrieHash.o" \
"CMakeFiles/devcore.dir/Account.o"

# External object files for target devcore
devcore_EXTERNAL_OBJECTS =

libdevcore.a: CMakeFiles/devcore.dir/Address.o
libdevcore.a: CMakeFiles/devcore.dir/Common.o
libdevcore.a: CMakeFiles/devcore.dir/CommonData.o
libdevcore.a: CMakeFiles/devcore.dir/DBFactory.o
libdevcore.a: CMakeFiles/devcore.dir/FileSystem.o
libdevcore.a: CMakeFiles/devcore.dir/FixedHash.o
libdevcore.a: CMakeFiles/devcore.dir/LevelDB.o
libdevcore.a: CMakeFiles/devcore.dir/OverlayDB.o
libdevcore.a: CMakeFiles/devcore.dir/RLP.o
libdevcore.a: CMakeFiles/devcore.dir/SHA3.o
libdevcore.a: CMakeFiles/devcore.dir/StateCacheDB.o
libdevcore.a: CMakeFiles/devcore.dir/TrieCommon.o
libdevcore.a: CMakeFiles/devcore.dir/TrieHash.o
libdevcore.a: CMakeFiles/devcore.dir/Account.o
libdevcore.a: CMakeFiles/devcore.dir/build.make
libdevcore.a: CMakeFiles/devcore.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/ioc/DIONS/src/ptrie/CMakeFiles --progress-num=$(CMAKE_PROGRESS_15) "Linking CXX static library libdevcore.a"
	$(CMAKE_COMMAND) -P CMakeFiles/devcore.dir/cmake_clean_target.cmake
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/devcore.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/devcore.dir/build: libdevcore.a

.PHONY : CMakeFiles/devcore.dir/build

CMakeFiles/devcore.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/devcore.dir/cmake_clean.cmake
.PHONY : CMakeFiles/devcore.dir/clean

CMakeFiles/devcore.dir/depend:
	cd /home/ioc/DIONS/src/ptrie && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ioc/DIONS/src/ptrie /home/ioc/DIONS/src/ptrie /home/ioc/DIONS/src/ptrie /home/ioc/DIONS/src/ptrie /home/ioc/DIONS/src/ptrie/CMakeFiles/devcore.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/devcore.dir/depend

