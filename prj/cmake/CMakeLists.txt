cmake_minimum_required(VERSION 3.7)
project(Cpl)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include_directories(${ROOT_DIR}/src)

file(GLOB_RECURSE TEST_SRC ${ROOT_DIR}/src/Test/Test*.cpp)
add_executable(Test ${TEST_SRC})

target_link_libraries(Test -lpthread -lstdc++ -lstdc++fs)

if(UNIX)
target_link_libraries(Test -ldl)
endif()
