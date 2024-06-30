#define _CRT_SECURE_NO_WARNINGS

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char dns_db_file[256];
static int cache_size = 100;  // 默认缓存大小
static char log_level[10] = "info";  // 默认日志级别
static int server_port = 8053;  // 默认端口
static char external_dns_server[16] = "10.3.9.5"; // 默认外部DNS服务器

void config_load(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "dns_db_file=", 12) == 0) {
            strncpy(dns_db_file, buffer + 12, sizeof(dns_db_file) - 1);
            dns_db_file[sizeof(dns_db_file) - 1] = '\0';
            // 去掉末尾的换行符
            char* newline = strchr(dns_db_file, '\n');
            if (newline) {
                *newline = '\0';
            }
        } else if (strncmp(buffer, "cache_size=", 11) == 0) {
            cache_size = atoi(buffer + 11);
        } else if (strncmp(buffer, "log_level=", 10) == 0) {
            strncpy(log_level, buffer + 10, sizeof(log_level) - 1);
            log_level[sizeof(log_level) - 1] = '\0';
            // 去掉末尾的换行符
            char* newline = strchr(log_level, '\n');
            if (newline) {
                *newline = '\0';
            }
        } else if (strncmp(buffer, "server_port=", 12) == 0) {
            server_port = atoi(buffer + 12);
        }
        else if (strncmp(buffer, "external_dns_server=", 20) == 0) {
            strncpy(external_dns_server, buffer + 20, sizeof(external_dns_server) - 1);
            external_dns_server[sizeof(external_dns_server) - 1] = '\0';
            // 去掉末尾的换行符
            char* newline = strchr(external_dns_server, '\n');
            if (newline) {
                *newline = '\0';
            }
        }
    }

    fclose(file);
}

const char* config_get_dns_db_file() {
    return dns_db_file;
}

int config_get_cache_size() {
    return cache_size;
}

const char* config_get_log_level() {
    return log_level;
}

int config_get_server_port() {
    return server_port;
}

const char* config_get_external_dns_server() {
    return external_dns_server;
}