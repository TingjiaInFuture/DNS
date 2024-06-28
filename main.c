#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "thread_manager.h"
#include "log.h"

int main(int argc, char *argv[]) {
    // 初始化日志模块
    log_init();

    // 加载配置
   Config config;

    if (load_config("dnsrelay.txt", &config) != 0) {
        fprintf(stderr, "Failed to load configuration file.\n");
        return 1;
    }

    printf("Loaded %d DNS records:\n", config.record_count);
    for (int i = 0; i < config.record_count; i++) {
        printf("%s -> %s\n", config.records[i].domain, config.records[i].ip);
    }

    // 启动线程池
    if (start_thread_pool() != 0) {
        log_error("Failed to start thread pool.");
        return EXIT_FAILURE;
    }

    // 主线程等待
    while (1) {
        sleep(1);
    }

    // 清理资源
    stop_thread_pool();
    log_close();

    return EXIT_SUCCESS;
}
