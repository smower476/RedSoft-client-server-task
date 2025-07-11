#pragma once

#include <string>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef MAX_COMMAND_LEN
#define MAX_COMMAND_LEN 1024
#endif

#ifndef RECV_BUFFER_SIZE
#define RECV_BUFFER_SIZE 128
#endif

#ifndef SEND_BUFFER_SIZE
#define SEND_BUFFER_SIZE 128
#endif

#ifndef TIMEOUT_MS
#define TIMEOUT_MS 30000
#endif

/*
 * Пытается отправить весь буфер message в сокет sockfd 
 * @return true при успешной отправке всего сообщения, false при ошибке или таймауте
*/
bool safe_send(int sockfd, const std::string& message, int timeout_ms = TIMEOUT_MS);

/*
 * Считывает из сокета sock до символа '\n' (не включая '\r') в out.
 * Чтение прерывается при достижении MAX_COMMAND_LEN или при ошибке.
 * @return true при успешном чтении строки, false при ошибке, таймауте или переполнении
*/
bool recv_line(int sock, std::string& out, int timeout_ms = TIMEOUT_MS);
