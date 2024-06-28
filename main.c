#include "platform.h" // multiplatform socket initialization
#include "dns_query.h"
#include "config.h"
#include "cache.h"
#include "log.h"
#include "thread_manager.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void handle_dns_request(void *arg) {
    char *buffer = (char *)arg;
    char domain[256];
    if (!parse_dns_request(buffer, domain)) {
        log_error("Failed to parse DNS request");
        return;
    }

    char ip[16];
    if (lookup_domain_in_db(domain, ip)) {
        cache_insert(domain, ip);
        send_dns_response(buffer, ip);
    } else {
        log_error("Domain not found: %s", domain);
    }

    free(buffer);
}

int main() {
    if (!socket_init()) {
        log_error("Socket initialization failed");
        return 1;
    }

    dns_query_init("config.txt");
    thread_manager_init(4);

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
    serverAddr.sin_port = htons(53);

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        log_error("Bind failed");
        closesocket(sock);
        socket_cleanup();
        return 1;
    }

    while (1) {
        char *buffer = (char *)malloc(512);
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        int recvLen = recvfrom(sock, buffer, 512, 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (recvLen == SOCKET_ERROR) {
            log_error("recvfrom() failed");
            free(buffer);
            continue;
        }

        thread_manager_add_task(handle_dns_request, buffer);
    }

    closesocket(sock);
    socket_cleanup();
    dns_query_cleanup();
    thread_manager_destroy();
    return 0;
}
