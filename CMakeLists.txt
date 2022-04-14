cmake_minimum_required(VERSION 3.10)

# set the project name
project(testing)

set(SrcDir "code/src")

# GoogleTest requires at least C++11
if (CMAKE_CXX_STANDARD)
    if (${CMAKE_CXX_STANDARD} LESS 11)
        set(CMAKE_CXX_STANDARD 11)
    endif()
endif()

if (CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

message("CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
message("CMAKE_CXX_STANDARD: ${CMAKE_CXX_STANDARD}")
message("CMAKE_CXX_STANDARD_REQUIRED: ${CMAKE_CXX_STANDARD_REQUIRED}")

add_library(_some_code STATIC "${SrcDir}/some_code.cpp")
# add_library(_some_code SHARED "${SrcDir}/some_code.cpp")

# include(FetchContent)
# FetchContent_Declare(
#   googletest
#   URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip  # v1.11.0
# )
# # For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

# Assume that 'googletest' is installed in 'code/extern/googletest' directory
add_subdirectory(code/extern/googletest)

enable_testing()
add_executable(hello_test code/tests/test_hello.cpp)
target_link_libraries(hello_test gtest_main)
add_executable(test_some_code code/tests/test_some_code.cpp)
target_link_libraries(test_some_code _some_code gtest_main)
include(GoogleTest)
gtest_discover_tests(hello_test)
gtest_discover_tests(test_some_code)

# ==================== Build executable ===================
add_executable(testing "${SrcDir}/main.cpp")
target_link_libraries(testing _some_code)
