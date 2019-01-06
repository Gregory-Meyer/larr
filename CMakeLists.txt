cmake_minimum_required(VERSION 3.12)

project(larr)

set(CMAKE_C_STANDARD 90)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

find_package(Lua 5.3 REQUIRED)

include_directories(include/ ${LUA_INCLUDE_DIR})

add_compile_definitions(LUA_USE_C89)

add_library(larr SHARED src/larr.c src/util.c src/vec.c)
target_link_libraries(larr ${LUA_LIBRARIES})
