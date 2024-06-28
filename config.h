#ifndef CONFIG_H
#define CONFIG_H

#define MAX_ENTRIES 1000

typedef struct {
    char domain[256];
    char ip[16];
} DNSRecord;

// 定义一个配置结构体，存储DNS记录和记录数量
typedef struct {
    DNSRecord records[MAX_ENTRIES];
    int record_count;
} Config;

// 加载配置文件的函数声明
int load_config(const char *filename, Config *config);

#endif
