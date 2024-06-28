#define _CRT_SECURE_NO_WARNINGS

#include "dns_query.h"
#include "config.h"
#include "cache.h"
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
}

void dns_query_cleanup() {
    // 清理资源
}

int parse_dns_request(const char* request, char* domain) {
    // 解析 DNS 请求（简单模拟）
    strcpy(domain, request);
    return 1;
}

int lookup_domain_in_db(const char* domain, char* ip) {
    FILE* file = fopen(dns_db_file, "r");
    if (file == NULL) {
        perror("Failed to open DNS DB file");
        return 0;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        char file_domain[256];
        char file_ip[16];
        if (sscanf(line, "%255s %15s", file_domain, file_ip) == 2) {
            if (strcmp(domain, file_domain) == 0) {
                strcpy(ip, file_ip);
                fclose(file);
                return 1;
            }
        }
    }

    fclose(file);
    return 0;
}

void send_dns_response(const char* domain, const char* ip) {
    // 发送 DNS 响应（简单模拟）
    printf("Domain: %s, IP: %s\n", domain, ip);
}

void dns_query_handle_request() {
    char request[256];
    char domain[256];
    char ip[16];

    // 读取请求（简单模拟）
    printf("Enter domain to query: ");
    scanf("%255s", request);

    // 解析请求
    if (!parse_dns_request(request, domain)) {
        log_error("Failed to parse DNS request");
        return;
    }

    // 查找缓存
    const char* cached_ip = cache_lookup(domain);
    if (cached_ip) {
        log_debug("Cache hit for domain %s", domain);
        send_dns_response(domain, cached_ip);
        return;
    }

    // 查找数据库
    if (lookup_domain_in_db(domain, ip)) {
        log_debug("DB hit for domain %s", domain);
        cache_insert(domain, ip);
        send_dns_response(domain, ip);
    } else {
        log_error("Domain not found: %s", domain);
    }
}
