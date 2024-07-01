#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
unsigned socket_init() {
    WSADATA wsaData;
    // 初始化 Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
       log_error("WSAStartup failed");
       return 0;
    }
    return 1;
}

void socket_cleanup() {
    // 清理 Winsock
    WSACleanup();
}
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define closesocket close
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1
#define socket_init() // 在 Linux 上，这个宏为空
#define socket_cleanup()
#endif

#endif // PLATFORM_H