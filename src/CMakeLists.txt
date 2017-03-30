# CMake Header
cmake_minimum_required (VERSION 2.8)
project (OGSSim)

# Version Number
set (OGSSim_VERSION_MAJOR 1)
set (OGSSim_VERSION_MINOR 2)

# Modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ./)

# Includes
include_directories (../include ../lib/ceph-9.2.0/src ../lib/ceph-9.2.0/src/crush ../lib/ceph-9.2.0/src/include)

# Libraries
set (EXTRA_LIBS ${EXTRA_LIBS} glog gflags pthread xerces-c zmq zmqpp mgl cephfs)

# Sources
file (GLOB SRC *.cpp ComputationModels/*.cpp DecRAIDSchemes/*.cpp Drivers/*.cpp GraphGeneration/*.cpp LayoutModels/*.cpp Modules/*.cpp Structures/*.cpp Utils/*.cpp XMLParsers/*.cpp)

# Definitions
set (CMAKE_CXX_FLAGS_DEBUG ${CMAKE_CXX_FLAGS} "-std=gnu++11 -fext-numeric-literals -std=c++11 -O0 -g")
set (CMAKE_CXX_FLAGS_RELEASE ${CMAKE_CXX_FLAGS} "-std=gnu++11 -fext-numeric-literals -std=c++11 -g -O3 -DNDEBUG")

# Executable
message (STATUS ${EXTRA_LIBS})

add_executable (OGSSim ${SRC})
target_link_libraries (OGSSim ${EXTRA_LIBS})
install (TARGETS OGSSim DESTINATION .)

# CPack
set (CPACK_GENERATOR "DEB")
set (CPACK_DEBIAN_PACKAGE_MAINTAINER "Sebastien Gougeaud")
include (CPack)
