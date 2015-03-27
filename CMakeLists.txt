cmake_minimum_required (VERSION 2.6)
project (SUBSET_SUM)
# The version number.
set (SUBSET_SUM_VERSION_MAJOR 0)
set (SUBSET_SUM_VERSION_MINOR 12)

set(PLATFORM 64)

set(CMAKE_CXX_FLAGS "-g -Wall -O3 -funroll-loops -msse3")

#This may need to be updated or Linux/Windows
set(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} /opt/local/lib)

include_directories(${PROJECT_SOURCE_DIR})
#message("project source dir = ${PROJECT_SOURCE_DIR}")

#This will look for the undvc_common library, which should be in the
#same directory as TAO. It should be compiled in the ../undvc_common/build
#directory
FIND_PATH(UNDVC_COMMON_INCLUDE_DIR file_io.hxx
    ../undvc_common         #add addiitonal paths here if undvc_common is in a different directory
    )

MESSAGE(STATUS "UNDVC_COMMON include directory: ${UNDVC_COMMON_INCLUDE_DIR}")

FIND_LIBRARY(UNDVC_COMMON_LIBRARY
    NAMES undvc_common
    PATHS ../undvc_common/build         #add additional paths here if undvc_common is in a different directory
                                        #or if it has been build in a different directory
    PATH_SUFFIXES lib
    )
MESSAGE(STATUS "UNDVC_COMMON library: ${UNDVC_COMMON_LIBRARY}")

include_directories(${UNDVC_COMMON_INCLUDE_DIR})

#UNDVC_COMMON has the FindBOINC.cmake and FindMYSQL.cmake files
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${UNDVC_COMMON_INCLUDE_DIR}/cmake/Modules/")

find_package(BOINC)

#
#   Just in case we need Boost
#
#set(BOOST_LIB_PREFIX "lib")
#set(Boost_LIB_PREFIX "lib")
#set(BOOST_COMPILER "-vc100")
#set(Boost_COMPILER "-vc100")
#set(Boost_ADDITIONAL_VERSIONS "1.47" "1.47.0")
#find_package(Boost REQUIRED system filesystem)
#include_directories (${Boost_INCLUDE_DIR})
#message("FOUND BOOST DIRECTORIES: ${Boost_INCLUDE_DIR}")

include_directories(/home/tdesell/boost_1_53_0/)

find_package(BOINC)

add_subdirectory(common)
#add_subdirectory(client)

find_package(MySQL)
find_package(Boost REQUIRED COMPONENTS system)


if (BOINC_SERVER_FOUND AND MYSQL_FOUND)
    link_directories(${Boost_LIBRARY_DIRS})
    add_subdirectory(server)
else (BOINC_SERVER_FOUND AND MYSQL_FOUND)
    message(STATUS "Either BOINC libraries (found = ${BOINC_SERVER_FOUND}) or MYSQL libraries (found = ${MYSQL_FOUND}) missing, not compiling server subdirectory.")
endif (BOINC_SERVER_FOUND AND MYSQL_FOUND)

