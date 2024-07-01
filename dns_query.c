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

// 构建DNS查询报文
void build_dns_query(const char* domain, char* query) {
    // 指向当前写入位置的指针
    char *qname = query + sizeof(DNSHeader);
    
    // 初始化报头
    DNSHeader *header = (DNSHeader *)query;
    header->id = htons(0x1234);       // 设置事务ID
    header->flags = htons(0x0100);    // 设置标志位（递归查询）
    header->qdcount = htons(1);       // 设置问题数为1
    header->ancount = 0;              // 设置回答数为0
    header->nscount = 0;              // 设置授权数为0
    header->arcount = 0;              // 设置附加数为0

    // 构建QNAME部分
    const char *label_start = domain;
    const char *label_end;
    while ((label_end = strchr(label_start, '.')) != NULL) {
        size_t label_len = label_end - label_start;
        *qname++ = label_len;
        memcpy(qname, label_start, label_len);
        qname += label_len;
        label_start = label_end + 1;
    }

    // 添加最后一个标签
    size_t label_len = strlen(label_start);
    *qname++ = label_len;
    memcpy(qname, label_start, label_len);
    qname += label_len;
    *qname++ = 0;  // QNAME 以0结尾

    // 设置查询部分
    DNSQuestion *question = (DNSQuestion *)qname;
    question->qtype = htons(0x0001);  // 查询类型A
    question->qclass = htons(0x0001); // 查询类IN
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
// 解析DNS响应
int parse_dns_respond(const char* respond, char* ip) {
    // 解析报头
    DNSHeader *header = (DNSHeader *)respond;
    uint16_t ancount = ntohs(header->ancount);

    // 跳过报头和问题部分
    const char *ptr = respond + sizeof(DNSHeader);

    // 跳过QNAME
    while (*ptr != 0) {
        ptr += *ptr + 1;
    }
    ptr += 1; // 跳过QNAME的结尾0字节

    // 跳过QTYPE和QCLASS
    ptr += 4;

    // 解析回答部分
    for (int i = 0; i < ancount; i++) {
        // 跳过NAME
        if ((*ptr & 0xC0) == 0xC0) {
            ptr += 2; // 压缩域名
        } else {
            while (*ptr != 0) {
                ptr += *ptr + 1;
            }
            ptr += 1; // 跳过NAME的结尾0字节
        }

        // 解析TYPE和CLASS
        uint16_t type = ntohs(*(uint16_t *)ptr);
        uint16_t class = ntohs(*(uint16_t *)(ptr + 2));
        ptr += 4;

        // 跳过TTL
        ptr += 4;

        // 解析RDLENGTH
        uint16_t rdlength = ntohs(*(uint16_t *)ptr);
        ptr += 2;

        // 解析RDATA
        if (type == 1 && class == 1 && rdlength == 4) { // A记录
            sprintf(ip, "%u.%u.%u.%u", (unsigned char)ptr[0], (unsigned char)ptr[1], (unsigned char)ptr[2], (unsigned char)ptr[3]);
            return 1;
        }

        // 跳过当前记录的RDATA
        ptr += rdlength;
    }

    return 0; // 没有找到A记录
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

int send_dns_query(const char* query, size_t query_len, char* response, size_t response_len) {
    const char* external_dns_server = config_get_external_dns_server();
    if (!external_dns_server) {
        fprintf(stderr, "Failed to get external DNS server address from config\n");
        return -1;
    }

    // 初始化Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return -1;
    }

    // 套接字相关变量
    SOCKET sockfd;
    struct sockaddr_in server_addr;

    // 创建UDP套接字
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed\n");
        WSACleanup();
        return -1;
    }

    // 清零并设置服务器地址
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(53); // DNS uses port 53

    // 将IP地址从点分十进制转换为二进制格式
    if (inet_pton(AF_INET, external_dns_server, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "inet_pton failed\n");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    // 发送DNS查询请求
    if (sendto(sockfd, query, query_len, 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        fprintf(stderr, "sendto failed\n");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    // 接收DNS响应
    struct sockaddr_in from_addr;
    int from_len = sizeof(from_addr);
    int n = recvfrom(sockfd, response, response_len, 0, (struct sockaddr*)&from_addr, &from_len);
    if (n == SOCKET_ERROR) {
        fprintf(stderr, "recvfrom failed\n");
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    // 关闭套接字
    closesocket(sockfd);
    WSACleanup();
    return n;
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
    else {  //todo!


    }

    free(buffer); 
}
