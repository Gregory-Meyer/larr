cmake_minimum_required(VERSION 3.8)

project(larr)

set(CMAKE_C_STANDARD 89)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Lua 5.3 REQUIRED)

include_directories(include/ ${LUA_INCLUDE_DIR})

add_library(larr SHARED src/array.c)
target_link_libraries(larr ${LUA_LIBRARIES})
