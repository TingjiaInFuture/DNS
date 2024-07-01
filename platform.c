#include "platform.h"
unsigned socket_init() {
    WSADATA wsaData;
    // ��ʼ�� Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log_error("WSAStartup failed");
        return 0;
    }
    return 1;
}

void socket_cleanup() {
    // ���� Winsock
    WSACleanup();
}