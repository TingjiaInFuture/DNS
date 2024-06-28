#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char dns_server_ip[16];
static char config_file[256];

int load_config(const char *filename) {
    char buffer[512];  // 定义buffer，用于存储从文件读取的一行数据

    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        // 解析配置内容
        if (strncmp(buffer, "DNS_SERVER=", 11) == 0) {
            strncpy(dns_server_ip, buffer + 11, sizeof(dns_server_ip) - 1);
            dns_server_ip[sizeof(dns_server_ip) - 1] = '\0'; // 确保字符串以'\0'结尾
        }
        // 其他配置解析
    }

    fclose(file);
    return 0;
}

const char *get_dns_server_ip() {
    return dns_server_ip;
}
