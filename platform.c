#include "platform.h"
#include "log.h"
#include <WinSock2.h>

#ifdef _WIN32

unsigned socket_init() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log_error("WSAStartup failed");
        return 0;
    }
    return 1;
}

SOCKET create_udp_socket() {
    socket_init();
    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        log_error("Failed to create socket");
    }
    return sockfd;
}

struct sockaddr_in create_server_address(const char* ip, int port) {
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(ip);
    return servaddr;
}


int send_dns_query(SOCKET sockfd, const struct sockaddr_in* servaddr, const char* request) {
    if (sendto(sockfd, request, strlen(request), 0, (struct sockaddr*)servaddr, sizeof(*servaddr)) == SOCKET_ERROR) {
        log_error("Failed to send request");
        return 0;
    }
    return 1;
}

int receive_dns_response(SOCKET sockfd, struct in_addr* response) {
    int numbytes = recvfrom(sockfd, response, 512, 0, NULL, NULL);
    if (numbytes == SOCKET_ERROR) {
        log_error("Failed to receive response");
        return 0;
    }
    return 1;
}

void socket_cleanup() {
    WSACleanup();
}

#endif
