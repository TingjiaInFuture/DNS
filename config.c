#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char upstream_dns_ip[16];
static int cache_size = 100;  // 默认缓存大小
static char log_level[10] = "info";  // 默认日志级别
static int server_port = 8053;  // 默认端口

void config_load(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(EXIT_FAILURE);
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), file)) {
        if (strncmp(buffer, "upstream_dns_ip=", 16) == 0) {
            strncpy(upstream_dns_ip, buffer + 16, sizeof(upstream_dns_ip) - 1);
            upstream_dns_ip[sizeof(upstream_dns_ip) - 1] = '\0';
            // 去掉末尾的换行符
            char* newline = strchr(upstream_dns_ip, '\n');
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
    }

    fclose(file);
}

const char* config_get_upstream_dns_ip() {
    return upstream_dns_ip;
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
