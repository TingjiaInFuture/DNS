#include "platform.h"
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