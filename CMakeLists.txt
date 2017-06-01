cmake_minimum_required(VERSION 2.8)
project(libibautils)

set(LIBIBAUTILS "ibautils")
set(CMAKE_BUILD_TYPE Release)

target_compile_features(libibautils PRIVATE cxx_range_for)
add_library(lib1 SHARED lib1.cpp)
set_property(TARGET lib1 PROPERTY POSITION_INDEPENDENT_CODE ON)

# RE2
SET(RE2_DIR ${PROJECT_SOURCE_DIR})
FIND_PACKAGE(RE2 REQUIRED)

INCLUDE_DIRECTORIES(${RE2_INCLUDE_DIR})

ADD_SUBDIRECTORY("src")
