cmake_minimum_required(VERSION 3.10)

add_library(common_lib STATIC
    src/validation.cpp
    src/network_utils.cpp
)

target_include_directories(common_lib PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

set_target_properties(common_lib PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(common_lib PRIVATE
    -Wall
    -Wextra
    -Wpedantic
    -Werror
)

add_library(Chat::Common ALIAS common_lib)

