cmake_minimum_required(VERSION 2.8)
project(libibautils)

set(LIBIBAUTILS "ibautils")
set(CMAKE_BUILD_TYPE Release)
set(Boost_USE_STATIC_LIBS OFF)

target_compile_features(libibautils PRIVATE cxx_range_for)



# RE2
SET(RE2_DIR ${PROJECT_SOURCE_DIR})
FIND_PACKAGE(RE2 REQUIRED)

INCLUDE_DIRECTORIES(${RE2_INCLUDE_DIR})

ADD_SUBDIRECTORY("src")
