cmake_minimum_required(VERSION 3.16)
project(uflp_u LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(uflp ${SOURCES})

target_include_directories(uflp PRIVATE src)
