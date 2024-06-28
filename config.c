#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"

int load_config(const char *filename, Config *config) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "dns_server", 10) == 0) {
            sscanf(line, "dns_server %s", config->dns_server);
        } else if (strncmp(line, "cache_size", 10) == 0) {
            sscanf(line, "cache_size %d", &config->cache_size);
        } else if (strncmp(line, "log_file", 8) == 0) {
            sscanf(line, "log_file %s", config->log_file);
        }
    }

    fclose(file);
    return 0;
}
