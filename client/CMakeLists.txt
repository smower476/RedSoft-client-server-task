add_executable(chat_client
    client.cpp
    src/connection.cpp
    src/commands.cpp
    src/InputValidation.cpp
)

target_include_directories(chat_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(chat_client PRIVATE
    Chat::Common
)
