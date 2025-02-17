#ifndef PLATFORM_H
#define PLATFORM_H
#include "log.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment (lib, "ws2_32.lib")
extern unsigned socket_init();
extern void socket_cleanup();
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define socket_init() (1) // 在 Linux 上，这个宏为空
#define socket_cleanup()
#endif

#endif // PLATFORM_H