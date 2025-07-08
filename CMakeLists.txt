cmake_minimum_required(VERSION 3.10)
project(Chat)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  

add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
