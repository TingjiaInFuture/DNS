#ifndef CONFIG_H
#define CONFIG_H

typedef struct {
    char dns_server[256];
    int cache_size;
    char log_file[256];
} Config;

int load_config(const char *filename, Config *config);

#endif
