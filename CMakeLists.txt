cmake_minimum_required(VERSION 2.6)
project(HLXexam)

set(DEBUG_COMPILE_FLAGS "-g")
set(OPTIMIZATIONS "-O3")

# Flags for debugging
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y ${DEBUG_COMPILE_FLAGS}")

# Flags for optimization:
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y ${OPTIMIZATIONS}")

# Flasg for "release build"

add_executable(mainParent mainParent.cpp)
add_executable(childA childA.cpp)
add_executable(childB childB.cpp)

add_executable(testComputations testComputations.cpp)

target_link_libraries(mainParent -lpthread -lrt)
target_link_libraries(childB -lpthread -lrt)
