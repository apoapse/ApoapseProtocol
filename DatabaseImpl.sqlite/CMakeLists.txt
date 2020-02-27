cmake_minimum_required(VERSION 3.4)
project(DatabaseImpl.sqlite LANGUAGES CXX)
set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")

include_directories("../Code/")
include_directories("../ThirdParty/sqlite/include/")

set(OUTPUT_RELEASE ../../Bin/)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")
set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${OUTPUT_RELEASE}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})
set(QLITE3_INCLUDE_DIR, "../ThirdParty/sqlite/include/")
set(SQLITE3_LIBRARIES, "../ThirdParty/sqlite/bin/")

include_directories("../ThirdParty/boost_1_71_0_LINUX/")

# Source Files
set(SRC_FILES
    ./SQLite.cpp
)
source_group("Sources" FILES ${SRC_FILES})

# Header Files
set(HEADERS_FILES
    SQLite.h
)
source_group("Headers" FILES ${HEADERS_FILES})
ADD_LIBRARY(${PROJECT_NAME} SHARED ${SRC_FILES} ${HEADERS_FILES})

target_link_libraries(${PROJECT_NAME} sqlite3)

set_property(TARGET DatabaseImpl.sqlite PROPERTY CXX_STANDARD 17)