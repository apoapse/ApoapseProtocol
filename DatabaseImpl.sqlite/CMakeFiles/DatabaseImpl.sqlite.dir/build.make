# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.13

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite

# Include any dependencies generated for this target.
include CMakeFiles/DatabaseImpl.sqlite.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/DatabaseImpl.sqlite.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/DatabaseImpl.sqlite.dir/flags.make

CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.o: CMakeFiles/DatabaseImpl.sqlite.dir/flags.make
CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.o: SQLite.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.o"
	clang++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.o -c /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite/SQLite.cpp

CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.i"
	clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite/SQLite.cpp > CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.i

CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.s"
	clang++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite/SQLite.cpp -o CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.s

# Object files for target DatabaseImpl.sqlite
DatabaseImpl_sqlite_OBJECTS = \
"CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.o"

# External object files for target DatabaseImpl.sqlite
DatabaseImpl_sqlite_EXTERNAL_OBJECTS =

/mnt/c/Apoapse/DEV/BinLinux/libDatabaseImpl.sqlite.so: CMakeFiles/DatabaseImpl.sqlite.dir/SQLite.cpp.o
/mnt/c/Apoapse/DEV/BinLinux/libDatabaseImpl.sqlite.so: CMakeFiles/DatabaseImpl.sqlite.dir/build.make
/mnt/c/Apoapse/DEV/BinLinux/libDatabaseImpl.sqlite.so: CMakeFiles/DatabaseImpl.sqlite.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library /mnt/c/Apoapse/DEV/BinLinux/libDatabaseImpl.sqlite.so"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/DatabaseImpl.sqlite.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/DatabaseImpl.sqlite.dir/build: /mnt/c/Apoapse/DEV/BinLinux/libDatabaseImpl.sqlite.so

.PHONY : CMakeFiles/DatabaseImpl.sqlite.dir/build

CMakeFiles/DatabaseImpl.sqlite.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/DatabaseImpl.sqlite.dir/cmake_clean.cmake
.PHONY : CMakeFiles/DatabaseImpl.sqlite.dir/clean

CMakeFiles/DatabaseImpl.sqlite.dir/depend:
	cd /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite /mnt/c/Apoapse/DEV/ApoapseCore/DatabaseImpl.sqlite/CMakeFiles/DatabaseImpl.sqlite.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/DatabaseImpl.sqlite.dir/depend

