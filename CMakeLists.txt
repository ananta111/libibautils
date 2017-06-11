cmake_minimum_required(VERSION 2.8)
project(libibautils)

set(LIBIBAUTILS "ibautils")
set(CMAKE_BUILD_TYPE Release)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC" ) 


add_definitions(-Wall -std=c++0x -fPIC)







# RE2
SET(RE2_DIR ${PROJECT_SOURCE_DIR})
FIND_PACKAGE(RE2 REQUIRED)

INCLUDE_DIRECTORIES(${RE2_INCLUDE_DIR})

ADD_SUBDIRECTORY("src")
