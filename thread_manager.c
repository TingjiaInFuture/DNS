#include "thread_manager.h"
#include "dns_query.h"
#include <pthread.h>

static pthread_t *thread_pool;
static int thread_count = 4;

void *thread_func(void *arg) {
    while (1) {
        // 处理客户端请求
    }
}

int start_thread_pool() {
    thread_pool = malloc(thread_count * sizeof(pthread_t));
    for (int i = 0; i < thread_count; i++) {
        pthread_create(&thread_pool[i], NULL, thread_func, NULL);
    }
    return 0;
}

void stop_thread_pool() {
    // 停止线程池
}
