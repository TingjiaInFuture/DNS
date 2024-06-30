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
        return sockfd;
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(0);  // 绑定到一个随机的端口
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  // 绑定到所有可用的接口

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        log_error("Failed to bind socket");
        closesocket(sockfd);
        return INVALID_SOCKET;
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

// 构造DNS请求报文
void build_dns_query(const char* domain, char* query, int* query_len) {
    dns_header_t header = { htons(12345), htons(0x0100), htons(1), 0, 0, 0 };
    memcpy(query, &header, sizeof(header));

    char* p = query + sizeof(header);
    const char* label = domain;
    while (1) {
        const char* dot = strchr(label, '.');
        if (dot == NULL) {
            dot = domain + strlen(domain);
        }
        unsigned char length = dot - label;
        *p++ = length;
        memcpy(p, label, length);
        p += length;
        if (*dot == '\0') {
            break;
        }
        label = dot + 1;
    }
    *p++ = 0;

    dns_question_t question = { "", htons(1), htons(1) };
    memcpy(p, &question.qtype, sizeof(question) - sizeof(question.qname));
    *query_len = p - query + sizeof(question) - sizeof(question.qname);
}

int send_dns_query(SOCKET sockfd, const struct sockaddr_in* servaddr, const char* request, int request_len) {
    if (sendto(sockfd, request, request_len, 0, (struct sockaddr*)servaddr, sizeof(*servaddr)) == SOCKET_ERROR) {
        log_error("Failed to send request");
        printf("%d", WSAGetLastError());
        return 0;
    }
    return 1;
}

//int receive_dns_response(SOCKET sockfd, struct in_addr* response) {
//    int numbytes = recvfrom(sockfd, response, 512, 0, NULL, NULL);
//    if (numbytes == SOCKET_ERROR) {
//        log_error("Failed to receive response");
//        return 0;+
//    }
//    return 1;
//}
// 解析DNS响应报文
int receive_dns_response(SOCKET sockfd, struct in_addr* response_ip) {
    char response[512];
    int numbytes = recvfrom(sockfd, response, sizeof(response) - 1, 0, NULL, NULL);
    if (numbytes == SOCKET_ERROR) {
        log_error("Failed to receive response");
        return 0;
    }
    response[numbytes] = '\0';  // 添加null字符

    // 跳过DNS响应报文的头部和问题部分
    char* p = response + sizeof(dns_header_t) + strlen(response + sizeof(dns_header_t)) + 1 + sizeof(dns_question_t);

    // 解析第一个资源记录
    dns_resource_record_t rr;
    rr.name = p;
    p += strlen(p) + 1;
    rr.type = ntohs(*(unsigned short*)p);
    p += 2;
    rr.class = ntohs(*(unsigned short*)p);
    p += 2;
    rr.ttl = ntohl(*(unsigned int*)p);
    p += 4;
    rr.rdlength = ntohs(*(unsigned short*)p);
    p += 2;
    rr.rdata = p;

    // 如果资源记录的类型是A记录，那么资源数据就是IP地址
    if (rr.type == 1) {
        memcpy(response_ip, rr.rdata, sizeof(*response_ip));
    }
    return 1;
}


void socket_cleanup() {
    WSACleanup();
}

#endif
