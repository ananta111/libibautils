# - Try to find RE2
# Once done this will define
#  
#  RE2_FOUND        - system has RE2
#  RE2_INCLUDE_DIR  - the RE2 include directory
#  RE2_LIBRARY      - Link these to use RE2

FIND_PATH(RE2_INCLUDE_DIR NAMES re2/re2.h)
FIND_LIBRARY(RE2_LIBRARY re2 NAMES re2 re2.a re2.so re2.dynlib )

if(RE2_INCLUDE_DIR AND RE2_LIBRARY)
    MESSAGE(STATUS "Found RE2: ${RE2_LIBRARY}")
    SET(RE2_FOUND TRUE)

    ADD_LIBRARY(re2 SHARED IMPORTED)
    set_target_properties(re2 PROPERTIES IMPORTED_LOCATION ${RE2_LIBRARY})

else(RE2_INCLUDE_DIR AND RE2_LIBRARY)
    MESSAGE(FATAL_ERROR "Unable to find RE2 library. Please install RE2: https://github.com/google/re2" )
    SET(RE2_FOUND FALSE)
endif(RE2_INCLUDE_DIR AND RE2_LIBRARY)

