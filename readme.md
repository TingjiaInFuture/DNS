根据你提供的实验指导书，我们需要设计一个 DNS 中继服务器程序。以下是根据指导书要求的C语言方案，包括子程序划分、单一功能子程序、多线程关系和面向对象的设计思想：

### 1. 程序总体结构

- **主程序 (main.c)**: 负责程序的初始化、加载配置文件、启动多线程服务器。
- **配置加载模块 (config.c)**: 负责加载和解析配置文件。
- **DNS 查询模块 (dns_query.c)**: 负责处理 DNS 查询，包括本地查询和中继查询。
- **缓存模块 (cache.c)**: 负责实现 DNS 查询结果的缓存，使用 LRU（最近最少使用）算法。
- **多线程管理模块 (thread_manager.c)**: 负责创建和管理处理客户端请求的线程。
- **日志模块 (log.c)**: 负责记录调试和运行日志。

### 2. 各子程序的功能

- **主程序 (main.c)**:
  - 初始化系统资源（如加载配置、初始化日志模块等）。
  - 启动多线程服务器，监听客户端的 DNS 请求。
  - 清理资源并退出。

- **配置加载模块 (config.c)**:
  - 读取并解析配置文件，获取 DNS 服务器地址、缓存大小等配置信息。

- **DNS 查询模块 (dns_query.c)**:
  - 处理来自客户端的 DNS 查询请求。
  - 在本地表中查找域名对应的 IP 地址。
  - 如果未找到，向上级 DNS 服务器发送查询请求。
  - 返回查询结果给客户端。

- **缓存模块 (cache.c)**:
  - 实现 LRU 缓存算法，存储最近的 DNS 查询结果。
  - 提供缓存查询、更新和删除的接口。

- **多线程管理模块 (thread_manager.c)**:
  - 创建线程池，管理线程的创建、销毁。
  - 分配客户端请求到不同的线程进行处理。

- **日志模块 (log.c)**:
  - 提供日志记录功能，包括调试信息和运行日志。

### 3. 多线程关系

- **主线程**: 负责初始化系统和启动监听线程。
- **监听线程**: 负责监听客户端请求，并将请求分配给工作线程。
- **工作线程**: 从线程池中获取，负责具体处理客户端的 DNS 请求，包括本地查询和中继查询。

### 4. 面向对象的设计思想

尽管 C 语言不是面向对象的语言，但可以通过模块化设计和抽象接口来模拟面向对象的编程思想：

- **抽象接口**: 每个模块提供一组操作接口，隐藏内部实现细节。例如，缓存模块提供 `cache_lookup`、`cache_insert` 等接口。
- **模块化设计**: 每个功能模块相对独立，模块之间通过接口进行交互，减少耦合度。
- **数据封装**: 使用 `struct` 来封装数据和相关操作。例如，DNS 查询模块可以定义 `struct DNSQuery`，封装查询信息和处理函数。

### 5. 代码示例

**main.c**:
```c
#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "thread_manager.h"
#include "log.h"

int main(int argc, char *argv[]) {
    // 初始化日志模块
    log_init();

    // 加载配置
    if (load_config("dnsrelay.conf") != 0) {
        log_error("Failed to load configuration.");
        return EXIT_FAILURE;
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
```

**config.c**:
```c
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char dns_server_ip[16];
static char config_file[256];

int load_config(const char *filename) {
    // 简单的配置文件读取示例
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        return -1;
    }

    while (fgets(buffer, sizeof(buffer), file)) {
        // 解析配置内容
    }

    fclose(file);
    return 0;
}

const char *get_dns_server_ip() {
    return dns_server_ip;
}
```

**dns_query.c**:
```c
#include "dns_query.h"
#include "cache.h"
#include "log.h"

void handle_dns_query(int client_socket) {
    // 处理 DNS 查询请求
    // 从缓存中查找
    // 向上级 DNS 服务器查询
    // 返回查询结果
}
```

**cache.c**:
```c
#include "cache.h"
#include <stdlib.h>
#include <string.h>

typedef struct CacheEntry {
    char domain[256];
    char ip[16];
    struct CacheEntry *next;
} CacheEntry;

static CacheEntry *cache = NULL;

void cache_init() {
    // 初始化缓存
}

const char *cache_lookup(const char *domain) {
    // 查找缓存
}

void cache_insert(const char *domain, const char *ip) {
    // 插入缓存
}
```

**thread_manager.c**:
```c
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
```

**log.c**:
```c
#include "log.h"
#include <stdio.h>
#include <stdarg.h>

void log_init() {
    // 初始化日志模块
}

void log_close() {
    // 关闭日志模块
}

void log_debug(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stdout, format, args);
    va_end(args);
}

void log_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
}
```

### 6. 注意事项

- **线程安全**: 确保多线程访问共享资源（如缓存）时的线程安全。
- **错误处理**: 考虑各种可能的错误情况，并进行适当处理。
- **性能优化**: 例如，优化缓存查找算法，提高查询速度。

### 7. 课程设计报告

按照指导书中的要求，撰写课程设计报告，包括系统功能设计、模块划分和软件流程图、测试用例及运行结果、调试中遇到的问题和解决方案、总结和心得体会等。

通过以上设计，可以实现一个符合要求的 DNS 中继服务器程序。具体实现细节可以根据实验需求进行调整和完善。