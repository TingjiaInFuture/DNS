#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "dns_query.h"
#include "config.h"
#include "cache.h"
#include "platform.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char dns_db_file[256];

void dns_query_init(const char* config_file) {
    config_load(config_file);  // 加载配置文件
    const char* db_file = config_get_dns_db_file();
    if (db_file == NULL) {
        log_error("Failed to get DNS DB file from config");
        exit(EXIT_FAILURE);
    }
    strncpy(dns_db_file, db_file, sizeof(dns_db_file) - 1);
    dns_db_file[sizeof(dns_db_file) - 1] = '\0';
    log_debug("DNS DB file: %s", dns_db_file);
}

void dns_query_cleanup() {//todo!
    // 清理资源
}

int parse_dns_request(const char* request, char* domain) {
    int offset = 12; // 跳过12字节的报头部分
    int domain_len = 0;

    while (request[offset] != 0) {
        // 读取当前标签的长度
        int label_len = request[offset];
        offset++;
        
        // 将当前标签复制到域名缓冲区中
        for (int i = 0; i < label_len; i++) {
            domain[domain_len++] = request[offset++];
        }

        // 在标签之间添加一个点
        domain[domain_len++] = '.';
    }

    // 移除最后一个多余的点并添加字符串结束符
    if (domain_len > 0) {
        domain[--domain_len] = '\0';
    }

    log_debug("Parsed domain: %s", domain);
    return 1;
}

int lookup_domain_in_db(const char* domain, char* ip) {
    FILE* file = fopen(dns_db_file, "r");
    if (file == NULL) {
        log_error("Failed to open DNS DB file");
        log_error("Failed to open DNS DB file: %s", dns_db_file);
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char file_domain[256];
        char file_ip[16];
        log_debug("Read line: %s", line);
        if (sscanf(line, "%255s %15s", file_domain, file_ip) == 2) {
            log_debug("Parsed domain: %s, IP: %s", file_domain, file_ip);
            if (strcmp(domain, file_domain) == 0) {
                strcpy(ip, file_ip);
                fclose(file);
                if (strcmp(ip, "0.0.0.0") == 0) {
                    log_debug("Domain not found in DB: %s", domain);
                    return -1;  // 域名不存在
                }
                log_debug("Found IP for domain %s: %s", domain, ip);
                return 1;
            }
        }
    }

    fclose(file);
    log_debug("Domain not found in DB: %s", domain);
    return 0;
}


void send_dns_response(const char* buffer, const char* ip) {//todo!
    // 发送 DNS 响应（简单模拟）
    log_debug("Sending DNS response: %s -> %s", buffer, ip);
    printf("Domain: %s, IP: %s\n", buffer, ip);
}

void dns_query_handle_request(void* arg) {
    char* buffer = (char*)arg;  // 从参数获取DNS请求数据
    char domain[256];
    if (!parse_dns_request(buffer, domain)) {  // 解析DNS请求，提取域名
        log_error("Failed to parse DNS request");
        free(buffer);  // 解析失败，释放内存
        return;
    }

    char ip[16];
    const char* cached_ip = cache_lookup(domain);
    if (cached_ip) {
        log_debug("Cache hit for domain %s", domain);
        send_dns_response(buffer, cached_ip);
        free(buffer);
        return;
    }

    int lookup_result = lookup_domain_in_db(domain, ip);  // 在本地数据库中查找域名对应的IP地址
    if (lookup_result == 1) {  // 找到普通IP地址
        cache_insert(domain, ip);  // 将结果插入缓存
        send_dns_response(buffer, ip);  // 发送DNS响应
    }
    else if (lookup_result == -1) {  // 找到IP地址为0.0.0.0，表示域名不存在
        send_dns_response(buffer, "0.0.0.0"); 
    }
    else {  
        log_debug("Cache miss for domain %s", domain);
        // 发送外部DNS查询请求
        const char* external_dns_server = config_get_external_dns_server();
        char request_datagram[512];
        struct in_addr response_ip;
        SOCKET sockfd = create_udp_socket();
        if (sockfd == INVALID_SOCKET) {
            free(buffer);  // 创建套接字失败，释放内存
            return;
        }
        struct sockaddr_in servaddr = create_server_address(external_dns_server, 53);
        int request_len;
        build_dns_query(domain, request_datagram, &request_len);  // 构建DNS查询报文
        if (!send_dns_query(sockfd, &servaddr, request_datagram, request_len)) {
            closesocket(sockfd);  // 发送查询失败，关闭套接字
            free(buffer);  
            return;
        }
        if (!receive_dns_response(sockfd, &response_ip)) {  // 接收DNS响应，超时设置为2000毫秒
            log_error("Failed to receive response for domain: %s", domain);
            closesocket(sockfd); 
            free(buffer);  
            return;
        }
        char external_dns_ip[16];
        strcpy(external_dns_ip, inet_ntoa(response_ip));  // 将二进制的IP地址转换为字符串
        cache_insert(domain, external_dns_ip);  // 将外部查询结果插入缓存
        send_dns_response(buffer, external_dns_ip);  // 发送DNS响应
        closesocket(sockfd);  // 关闭套接字
    }

    free(buffer); 
}
