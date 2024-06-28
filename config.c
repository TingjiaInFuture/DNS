#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

// 实现加载配置文件的函数
int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        // 打开配置文件失败，返回-1
        return -1;
    }

    char line[256];
    config->record_count = 0;

    // 逐行读取配置文件
    while (fgets(line, sizeof(line), file)) {
        if (config->record_count >= MAX_ENTRIES) {
            // 超过最大记录数限制
            break;
        }

        // 解析每行中的IP地址和域名
        if (sscanf(line, "%15s %255s", config->records[config->record_count].ip, config->records[config->record_count].domain) == 2) {
            config->record_count++;
        }
    }

    // 关闭配置文件
    fclose(file);
    return 0;
}
