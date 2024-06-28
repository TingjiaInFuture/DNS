#include "dns_query.h"
#include "cache.h"
#include "config.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#define DNS_PORT 53
#define BUFFER_SIZE 512

static char upstream_dns_ip[16];
static int dns_socket;
static int server_port;

void dns_query_init(const char* config_file) {
    config_load(config_file);  // 加载配置文件
    const char* upstream_ip = config_get_upstream_dns_ip();
    if (upstream_ip == NULL) {
        log_error("Failed to get upstream DNS IP from config");
        exit(EXIT_FAILURE);
    }
    strncpy(upstream_dns_ip, upstream_ip, sizeof(upstream_dns_ip) - 1);
    upstream_dns_ip[sizeof(upstream_dns_ip) - 1] = '\0';
    
    dns_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (dns_socket < 0) {
        log_error("Failed to create DNS socket");
        exit(EXIT_FAILURE);
    }

    server_port = config_get_server_port();
}

void dns_query_cleanup() {
    close(dns_socket);
}

void dns_query_handle_request(int client_socket) {
    unsigned char buffer[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    int n = recvfrom(client_socket, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &addr_len);

    if (n < 0) {
        log_error("Failed to receive data from client");
        return;
    }

    char domain[256] = {0};
    // 解析 DNS 请求获取域名
    if (!parse_dns_request(buffer, n, domain)) {
        log_error("Failed to parse DNS request");
        return;
    }

    // 从缓存中查找
    const char* ip = cache_lookup(domain);
    if (ip) {
        log_debug("Cache hit for domain %s", domain);
        send_dns_response(client_socket, buffer, n, ip, &client_addr, addr_len);
        return;
    }

    // 缓存未命中，向上游 DNS 服务器查询
    struct sockaddr_in upstream_addr;
    upstream_addr.sin_family = AF_INET;
    upstream_addr.sin_port = htons(DNS_PORT);
    inet_pton(AF_INET, upstream_dns_ip, &upstream_addr.sin_addr);

    if (sendto(dns_socket, buffer, n, 0, (struct sockaddr*)&upstream_addr, sizeof(upstream_addr)) < 0) {
        log_error("Failed to send data to upstream DNS server");
        return;
    }

    n = recvfrom(dns_socket, buffer, BUFFER_SIZE, 0, NULL, NULL);
    if (n < 0) {
        log_error("Failed to receive data from upstream DNS server");
        return;
    }

    // 解析上游 DNS 服务器的响应并更新缓存
    if (parse_dns_response(buffer, n, domain, ip)) {
        cache_insert(domain, ip);
    }

    // 将响应发送给客户端
    send_dns_response(client_socket, buffer, n, ip, &client_addr, addr_len);
}

int parse_dns_request(unsigned char* buffer, int size, char* domain) {
    int pos = 12;  // 跳过报头部分
    int length = buffer[pos++];
    while (length > 0 && pos < size) {
        strncat(domain, (char*)&buffer[pos], length);
        pos += length;
        length = buffer[pos++];
        if (length > 0) {
            strncat(domain, ".", 1);
        }
    }
    return 1;
}

int parse_dns_response(unsigned char* buffer, int size, char* domain, char* ip) {
    int pos = 6;  // 回答段数量字段的位置
    int answer_count = (buffer[pos] << 8) + buffer[pos + 1];
    pos = 12;
    
    // 跳过问题部分
    while (buffer[pos] != 0) {
        pos++;
    }
    pos += 5;
    
    // 解析回答部分
    for (int i = 0; i < answer_count; i++) {
        pos += 2;  // 跳过名字字段
        pos += 2;  // 跳过类型字段
        pos += 2;  // 跳过类字段
        pos += 4;  // 跳过TTL字段
        int data_length = (buffer[pos] << 8) + buffer[pos + 1];
        pos += 2;
        
        if (data_length == 4) {
            sprintf(ip, "%d.%d.%d.%d", buffer[pos], buffer[pos + 1], buffer[pos + 2], buffer[pos + 3]);
            return 1;
        }
        pos += data_length;
    }
    return 0;
}

void send_dns_response(int client_socket, unsigned char* buffer, int size, const char* ip, struct sockaddr_in* client_addr, socklen_t addr_len) {
    // 构建回答部分
    int pos = size;
    buffer[pos++] = 0xc0;  // 指向问题部分的偏移量
    buffer[pos++] = 0x0c;
    buffer[pos++] = 0x00;  // 类型A
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x00;  // 类IN
    buffer[pos++] = 0x01;
    buffer[pos++] = 0x00;  // TTL
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x00;
    buffer[pos++] = 0x3c;
    buffer[pos++] = 0x00;  // 数据长度4字节
    buffer[pos++] = 0x04;
    
    // IP 地址
    unsigned int ip_parts[4];
    sscanf(ip, "%u.%u.%u.%u", &ip_parts[0], &ip_parts[1], &ip_parts[2], &ip_parts[3]);
    for (int i = 0; i < 4; i++) {
        buffer[pos++] = (unsigned char)ip_parts[i];
    }
    
    // 更新回答段数量
    buffer[6] = 0x00;
    buffer[7] = 0x01;

    // 发送响应
    sendto(client_socket, buffer, pos, 0, (struct sockaddr*)client_addr, addr_len);
}
