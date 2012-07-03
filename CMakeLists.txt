cmake_minimum_required (VERSION 2.6)
project (SSS)
# The version number.
set (SSS_VERSION_MAJOR 1)
set (SSS_VERSION_MINOR 0)
 
set(CMAKE_CXX_FLAGS "-g -Wall -O3 -funroll-loops -ftree-vectorize -msse3")

set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/undvc_common/cmake/Modules/")

find_package(BOINC)
find_package(MySQL)
find_package(Boost REQUIRED COMPONENTS system)

#include_directories(undvc_common)

add_subdirectory(undvc_common)

link_directories(${Boost_LIBRARY_DIRS})

if (BOINC_FOUND AND MYSQL_FOUND)
    add_subdirectory(server)
else (BOINC_FOUND AND MYSQL_FOUND)
    message(STATUS "Either BOINC libraries (found = ${BOINC_FOUND}) or MYSQL libraries (found = ${MYSQL_FOUND}) missing, not compiling server subdirectory.")
endif (BOINC_FOUND AND MYSQL_FOUND)
