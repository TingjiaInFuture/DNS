#include "platform.h"

#ifdef _WIN32
unsigned socket_init() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log_error("WSAStartup failed");
        return 0;
    }
    return 1;
}

void socket_cleanup() {
    WSACleanup();
}
#endif