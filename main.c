#include "platform.h" // multiplatform socket initialization
#include "dns_query.h"
#include "config.h"
#include "cache.h"
#include "log.h"
#include "thread_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>



int main(int argc, char* argv[]) {
    char* debugOption = NULL;
    char* dnsServer = NULL;
    char* filePath = "config.txt"; // 默认配置文件
    int detailedDebug = 0;
    int basicDebug = 0;

    if (argc > 1) {
        debugOption = argv[1];
        if (argc > 2) {
            dnsServer = argv[2];
            if (argc > 3) {
                filePath = argv[3];
            }
        }
    }

    if (debugOption != NULL) {
        if (strcmp(debugOption, "-d") == 0) {
            printf("choose -d option.\n");
            basicDebug = 1;
        }
        else if (strcmp(debugOption, "-dd") == 0) {
            printf("choose -dd option.\n");
            detailedDebug = 1;
        }
        else {
            printf("Invalid option: %s\n", debugOption);
            return 1;
        }
    }

    printf("Using DNS server: %s\n", dnsServer ? dnsServer : config_get_external_dns_server());
    printf("Config file: %s\n", filePath);

    // 加载配置文件
    config_load(filePath);

    // 初始化日志
    log_init();

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

    dns_query_init(filePath);

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
    if (detailedDebug == 1) {
        console_log_detail("Server started on port %d", config_get_server_port());
    }
    int sequence_number = 0;

    while (1) {
        char* buffer = (char*)malloc(sizeof(SOCKET) + 512);
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        int recvLen = recvfrom(sock, buffer + sizeof(SOCKET), 512, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (recvLen == SOCKET_ERROR) {
            log_error("recvfrom() failed");
            free(buffer);
            continue;
        }

        char client_ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), client_ip, INET_ADDRSTRLEN);

        char query_domain[256];
        if (!parse_dns_request(buffer + sizeof(SOCKET), query_domain, detailedDebug)) {
            log_error("Failed to parse DNS request");
            free(buffer);
            continue;
        }

        log_debug("Received request from client %s for domain %s", client_ip, query_domain);

        if (detailedDebug) {
            console_log_detail("Received request from client %s for domain %s", client_ip, query_domain);
        }
        else if (basicDebug) {
            console_log_basic(client_ip, query_domain, sequence_number);
        }
        sequence_number++;

        memcpy(buffer, &sock, sizeof(SOCKET));

        struct request_data {
            SOCKET sock;
            char buffer[512];
            struct sockaddr_in clientAddr;
            int clientAddrLen;
        };

        struct request_data* data = malloc(sizeof(struct request_data));
        data->sock = sock;
        memcpy(data->buffer, buffer + sizeof(SOCKET), 512);
        data->clientAddr = clientAddr;
        data->clientAddrLen = clientAddrLen;
        free(buffer);

        thread_manager_add_task(dns_query_handle_request, data, detailedDebug);
    }

    closesocket(sock);
    socket_cleanup();
    dns_query_cleanup();
    thread_manager_destroy();
    log_close();

    return 0;
}
