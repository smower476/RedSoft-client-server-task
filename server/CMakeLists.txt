add_executable(chat_server
    src/main.cpp
    src/connections.cpp
    src/validation.cpp
    src/commands.cpp
)

target_include_directories(chat_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chat_server PRIVATE
    #common_lib
    pthread
)
