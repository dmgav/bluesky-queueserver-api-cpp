cmake_minimum_required(VERSION 3.10)

# set the project name
project(Testing)

set(SrcDir "code/src")

add_library(_some_code STATIC "${SrcDir}/some_code.cpp")
# add_library(_some_code SHARED "${SrcDir}/some_code.cpp")
add_executable(Testing "${SrcDir}/main.cpp")
target_link_libraries(Testing _some_code)
