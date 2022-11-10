cmake_minimum_required(VERSION 3.7)
project(Cpl)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include_directories(${ROOT_DIR}/src)

file(GLOB_RECURSE TEST_SRC ${ROOT_DIR}/src/Test/Test*.cpp)
#set_source_files_properties(${TEST_SRC} PROPERTIES COMPILE_FLAGS "${COMMON_CXX_FLAGS} -g")
add_executable(Test ${TEST_SRC})
target_link_libraries(Test -ldl -lpthread)

