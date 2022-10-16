cmake_minimum_required(VERSION 3.10)

# set the project name
project(bluesky-qserver-api-cpp LANGUAGES CXX)

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

# add_subdirectory(code/extern/json)

find_package(Threads REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code/extern/json/single_include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code/extern/cppzmq)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code/extern/cpp-httplib)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/code/include)

add_library(_some_code STATIC "${SrcDir}/some_code.cpp")
# add_library(_some_code SHARED "${SrcDir}/some_code.cpp")
# target_include_directories(_some_code PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/code/extern/json/include")

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
add_executable(some-exec "${SrcDir}/main.cpp")
target_link_libraries(some-exec _some_code)

# ==================== Build examples =====================
add_executable(ex01_zmq_cm "${SrcDir}/examples/ex01_zmq_cm.cpp")
target_link_libraries(ex01_zmq_cm zmq Threads::Threads)

add_executable(ex02_http_cm "${SrcDir}/examples/ex02_http_cm.cpp")
target_link_libraries(ex02_http_cm Threads::Threads)

add_executable(ex03_auth_api "${SrcDir}/examples/ex03_auth_api.cpp")
target_link_libraries(ex03_auth_api Threads::Threads)
