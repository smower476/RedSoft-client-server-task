add_executable(chat_server
    server.cpp
    src/connections.cpp
    src/commands.cpp
)

target_include_directories(chat_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chat_server PRIVATE
    Chat::Common
    pthread
)
