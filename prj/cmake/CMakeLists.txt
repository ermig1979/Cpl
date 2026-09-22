cmake_minimum_required(VERSION 3.8)
project(Cpl)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../..)

include_directories(${ROOT_DIR}/src)

file(GLOB_RECURSE TEST_SRC ${ROOT_DIR}/src/Test/Test*.cpp)
add_executable(Test ${TEST_SRC})

option(CPL_SANITIZE "Build the tests with AddressSanitizer and UndefinedBehaviorSanitizer" OFF)
if(CPL_SANITIZE)
    # vptr checks are disabled: Param.h walks parameter fields through casts between unrelated types.
    # -O1 is required for stack-use-after-return detection (run with ASAN_OPTIONS=detect_stack_use_after_return=1).
    set(CPL_SANITIZE_FLAGS -fsanitize=address,undefined -fno-sanitize=vptr -fno-sanitize-recover=all -fno-omit-frame-pointer -O1 -g)
    target_compile_options(Test PRIVATE ${CPL_SANITIZE_FLAGS})
    target_link_libraries(Test ${CPL_SANITIZE_FLAGS})
endif()

option(CPL_SANITIZE_THREAD "Build the tests with ThreadSanitizer (cannot be combined with CPL_SANITIZE)" OFF)
if(CPL_SANITIZE_THREAD)
    if(CPL_SANITIZE)
        message(FATAL_ERROR "CPL_SANITIZE and CPL_SANITIZE_THREAD are mutually exclusive: AddressSanitizer and ThreadSanitizer cannot be linked together.")
    endif()
    set(CPL_SANITIZE_THREAD_FLAGS -fsanitize=thread -fno-omit-frame-pointer -O1 -g)
    target_compile_options(Test PRIVATE ${CPL_SANITIZE_THREAD_FLAGS})
    target_link_libraries(Test ${CPL_SANITIZE_THREAD_FLAGS})
endif()

# Negative compilation checks. Every source in src/Test/negative uses a macro of the library in a
# way the library has to reject, so the check fails when one of them builds. They are compiled one
# by one because a compilation error cannot be reported by the test executable.
option(CPL_NEGATIVE_CHECKS "Check that the sources of src/Test/negative are rejected by the compiler" ON)
if(CPL_NEGATIVE_CHECKS)
    file(GLOB CPL_NEGATIVE_SRC ${ROOT_DIR}/src/Test/negative/*.cpp)
    # Compile only: a link failure would count as a rejection, and a driver that does not link the
    # C++ standard library by itself (clang given as CMAKE_CXX_COMPILER) fails every link.
    set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
    set(CPL_NEGATIVE_BUILT)
    foreach(CPL_NEGATIVE_CASE ${CPL_NEGATIVE_SRC})
        get_filename_component(CPL_NEGATIVE_NAME ${CPL_NEGATIVE_CASE} NAME_WE)
        # The result of try_compile goes to the cache, so without this the check runs once ever.
        unset(CPL_NEGATIVE_RESULT_${CPL_NEGATIVE_NAME} CACHE)
        try_compile(CPL_NEGATIVE_RESULT_${CPL_NEGATIVE_NAME}
            ${CMAKE_BINARY_DIR}/negative/${CPL_NEGATIVE_NAME}
            SOURCES ${CPL_NEGATIVE_CASE}
            CMAKE_FLAGS "-DINCLUDE_DIRECTORIES=${ROOT_DIR}/src"
            # The library is written for C++11, the oldest standard it has to reject these sources in.
            CXX_STANDARD 11
            CXX_STANDARD_REQUIRED ON
            CXX_EXTENSIONS OFF)
        if(CPL_NEGATIVE_RESULT_${CPL_NEGATIVE_NAME})
            list(APPEND CPL_NEGATIVE_BUILT ${CPL_NEGATIVE_NAME})
        endif()
    endforeach()
    unset(CMAKE_TRY_COMPILE_TARGET_TYPE)
    if(CPL_NEGATIVE_BUILT)
        string(REPLACE ";" ", " CPL_NEGATIVE_BUILT_TEXT "${CPL_NEGATIVE_BUILT}")
        message(FATAL_ERROR "The compiler accepted sources of src/Test/negative that it has to reject: ${CPL_NEGATIVE_BUILT_TEXT}.")
    endif()
    list(LENGTH CPL_NEGATIVE_SRC CPL_NEGATIVE_COUNT)
    message(STATUS "Negative checks: ${CPL_NEGATIVE_COUNT} sources rejected by the compiler, as required.")
endif()

target_link_libraries(Test -lpthread -lstdc++ -lstdc++fs -lm)

if(UNIX)
target_link_libraries(Test -ldl)
endif()
