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
