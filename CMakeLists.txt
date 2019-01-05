cmake_minimum_required(VERSION 3.8)

project(larr)

find_package(Lua 5.3 REQUIRED)

include_directories(include/ ${LUA_INCLUDE_DIR})

add_library(larr SHARED src/array.c)
target_link_libraries(larr ${LUA_LIBRARIES})
