#pragma once

#include <string>
#include <poll.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef MAX_COMMAND_LEN
#define MAX_COMMAND_LEN 1024
#endif

#ifndef TIMEOUT_MS
#define TIMEOUT_MS 30000
#endif

/**
 * Пытается отправить весь буфер message по сокету sockfd с таймаутом в timeout_ms миллисекунд.
 * @param sockfd дескриптор сокета
 * @param message строка для отправки
 * @param timeout_ms таймаут в миллисекундах для каждого вызова poll
 * @return true при успешной отправке всего сообщения, false при ошибке или таймауте
 */
bool safe_send(int sockfd, const std::string& message, int timeout_ms = TIMEOUT_MS);

/**
 * Считывает из сокета sock до символа '\n' (не включая '\r') в out.
 * Чтение прерывается при достижении MAX_COMMAND_LEN или при ошибке.
 * @param sock дескриптор сокета
 * @param out строка для записи полученных символов
 * @param timeout_ms таймаут в миллисекундах для каждого вызова poll
 * @return true при успешном чтении строки, false при ошибке, таймауте или переполнении
 */
bool recvLine(int sock, std::string& out, int timeout_ms = TIMEOUT_MS);
