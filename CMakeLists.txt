cmake_minimum_required(VERSION 3.13)
project(test_cmake CXX)

list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
find_package(ROOT 6.28 CONFIG REQUIRED ${ROOT_REQUIRED_COMPONENTS})
include(${ROOT_USE_FILE})
message(STATUS "Found ROOT: ${ROOT_INCLUDE_DIRS}")
message("-- Required ROOT components are: ${ROOT_REQUIRED_COMPONENTS}")

add_subdirectory(src)
add_subdirectory(execute)
