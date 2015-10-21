cmake_minimum_required(VERSION 2.8)
project(libibautils)

set(CMAKE_BUILD_TYPE Release)


SET(LIBIBAUTILS "ibautils")

file(GLOB SRCS "*.cpp")

# RE2
SET(RE2_DIR ${PROJECT_SOURCE_DIR})
FIND_PACKAGE(RE2 REQUIRED)

#add_executable(foo foo.cpp)
#target_link_libraries(foo boo)

INCLUDE_DIRECTORIES(${RE2_INCLUDE_DIR})
ADD_LIBRARY(${LIBIBAUTILS} SHARED ${SRCS})
SET_TARGET_PROPERTIES(${LIBIBAUTILS} PROPERTIES VERSION 0.0.1 SOVERSION 0)

##INSTALL(TARGETS ${LIBIBAUTILS}
##  RUNTIME DESTINATION ${TULIP_DIR}/bin COMPONENT ${LIBIBAUTILS}
##  LIBRARY DESTINATION ${TULIP_DIR}/lib COMPONENT ${LIBIBAUTILS}
##  ARCHIVE DESTINATION ${TULIP_DIR}/lib COMPONENT ${LIBIBAUTILS})
##

set(INSTALL_BIN_DIR bin CACHE PATH "Installation directory for executables")
set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
set(INSTALL_INC_DIR include CACHE PATH "Installation directory for headers")
set(INSTALL_DATA_DIR share/${PROJECT_NAME} CACHE PATH "Installation
directory for data files")
