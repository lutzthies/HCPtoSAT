cmake_minimum_required (VERSION 2.6)
project (helga)

# add c++11 and optimization
set(CMAKE_CXX_FLAGS "-std=c++11 -O3")

# add threads
set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)
if(CMAKE_USE_PTHREADS_INIT)
    set(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")
endif()

# add our executable
add_executable(helga helga.cxx VarManager.cxx)
set_target_properties(helga PROPERTIES OUTPUT_NAME "helga.out")

# add pblib for encoding of constraints
find_library(PBLIB pblib ../../pblib/lib)
target_link_libraries(helga ${PBLIB})
target_link_libraries(helga ${CMAKE_THREAD_LIBS_INIT} )
