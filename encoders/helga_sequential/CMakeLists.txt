cmake_minimum_required (VERSION 2.6)
project (helga)
# the version number
set (helga_VERSION_MAJOR 1)
set (helga_VERSION_MINOR 0)
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")

# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")

# add our main executable
add_executable(helga helga.cxx)
set_target_properties(helga PROPERTIES OUTPUT_NAME "helga.out")
# add pblib for encoding of constraints
find_library(PBLIB pblib ../../pblib/lib)
target_link_libraries(helga ${PBLIB})
