#include "platform.h" // multiplatform socket initialization
#include "dns_query.h"
#include "config.h"
#include "cache.h"
#include "log.h"
#include "thread_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    // 初始化日志
    log_init();

    // 加载配置文件
    config_load("config.txt");

    // 初始化缓存
    int cache_size = config_get_cache_size();
    cache_init(cache_size);

    // 初始化线程池
    int num_threads = 4;  // 根据需求调整线程数量
    thread_manager_init(num_threads);

    // 初始化套接字
    if (!socket_init()) {
        log_error("Socket initialization failed");
        return 1;
    }

    dns_query_init("config.txt");

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        log_error("Failed to create socket");
        socket_cleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons_new(config_get_server_port());

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        log_error("Bind failed");
        closesocket(sock);
        socket_cleanup();
        return 1;
    }

    log_debug("Server started on port %d", config_get_server_port());

    while (1) {
        char* buffer = (char*)malloc(512);
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        int recvLen = recvfrom(sock, buffer, 512, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (recvLen == SOCKET_ERROR) {
            log_error("recvfrom() failed");
            free(buffer);
            continue;
        }

        log_debug("Received request from client");

        thread_manager_add_task(dns_query_handle_request, buffer);
    }

    closesocket(sock);
    socket_cleanup();
    dns_query_cleanup();
    thread_manager_destroy();
    log_close();
    return 0;
}
