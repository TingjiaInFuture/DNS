#include "dns_query.h"
#include "config.h"
#include "cache.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log_error("WSAStartup failed");
        return 1;
    }

    dns_query_init("config.txt");

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        log_error("Failed to create socket");
        WSACleanup();
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
        WSACleanup();
        return 1;
    }

    while (1) {
        char buffer[512];
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);

        int recvLen = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddr, &clientAddrLen);
        if (recvLen == SOCKET_ERROR) {
            log_error("recvfrom() failed");
            continue;
        }

        char domain[256];
        if (!parse_dns_request(buffer, domain)) {
            log_error("Failed to parse DNS request");
            continue;
        }

        char ip[16];
        if (lookup_domain_in_db(domain, ip)) {
            cache_insert(domain, ip);
            send_dns_response(sock, (struct sockaddr*)&clientAddr, clientAddrLen, buffer, ip);
        } else {
            log_error("Domain not found: %s", domain);
        }
    }

    closesocket(sock);
    WSACleanup();
    dns_query_cleanup();
    return 0;
}
