add_executable(chat_client
    src/main.cpp
    src/connection.cpp
    src/commands.cpp
    src/validation.cpp
)

target_include_directories(chat_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chat_client PRIVATE
    #common_lib
)
