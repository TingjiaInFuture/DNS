#define _CRT_SECURE_NO_WARNINGS

#include "cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 缓存条目结构体
typedef struct CacheEntry {
    char domain[256];
    char ip[16];
    struct CacheEntry* prev;
    struct CacheEntry* next;
    struct CacheEntry* hash_next;
} CacheEntry;

// 缓存管理结构体
typedef struct {
    int capacity;
    int size;
    CacheEntry** table;
    CacheEntry* head;
    CacheEntry* tail;
} Cache;

static Cache* cache = NULL;

// 哈希函数
static unsigned int hash(const char* str) {
    unsigned int hash = 0;
    while (*str) {
        hash = (hash << 5) + *str++;
    }
    return hash;
}

// 初始化缓存
void cache_init(int capacity) {
    cache = (Cache*)malloc(sizeof(Cache));
    cache->capacity = capacity;
    cache->size = 0;
    cache->table = (CacheEntry**)malloc(capacity * sizeof(CacheEntry*));
    for (int i = 0; i < capacity; i++) {
        cache->table[i] = NULL;
    }
    cache->head = NULL;
    cache->tail = NULL;
}

// 查找缓存条目
const char* cache_lookup(const char* domain) {
    unsigned int index = hash(domain) % cache->capacity;
    CacheEntry* entry = cache->table[index];
    while (entry) {
        if (strcmp(entry->domain, domain) == 0) {
            // 移动到链表头部 (LRU)
            if (entry != cache->head) {
                // 从当前的位置移除
                if (entry->prev) {
                    entry->prev->next = entry->next;
                }
                if (entry->next) {
                    entry->next->prev = entry->prev;
                }
                if (entry == cache->tail) {
                    cache->tail = entry->prev;
                }
                // 移动到头部
                entry->next = cache->head;
                entry->prev = NULL;
                if (cache->head) {
                    cache->head->prev = entry;
                }
                cache->head = entry;
                if (cache->tail == NULL) {
                    cache->tail = entry;
                }
            }
            return entry->ip;
        }
        entry = entry->hash_next;
    }
    return NULL;
}

// 插入缓存条目
void cache_insert(const char* domain, const char* ip) {
    unsigned int index = hash(domain) % cache->capacity;
    CacheEntry* entry = cache->table[index];
    while (entry) {
        if (strcmp(entry->domain, domain) == 0) {
            // 更新 IP 并移动到链表头部 (LRU)
            strncpy(entry->ip, ip, sizeof(entry->ip) - 1);
            entry->ip[sizeof(entry->ip) - 1] = '\0';
            if (entry != cache->head) {
                // 从当前的位置移除
                if (entry->prev) {
                    entry->prev->next = entry->next;
                }
                if (entry->next) {
                    entry->next->prev = entry->prev;
                }
                if (entry == cache->tail) {
                    cache->tail = entry->prev;
                }
                // 移动到头部
                entry->next = cache->head;
                entry->prev = NULL;
                if (cache->head) {
                    cache->head->prev = entry;
                }
                cache->head = entry;
                if (cache->tail == NULL) {
                    cache->tail = entry;
                }
            }
            return;
        }
        entry = entry->hash_next;
    }
    // 如果缓存已满，删除尾部条目
    if (cache->size == cache->capacity) {
        CacheEntry* to_remove = cache->tail;
        cache->tail = to_remove->prev;
        if (cache->tail) {
            cache->tail->next = NULL;
        }
        unsigned int remove_index = hash(to_remove->domain) % cache->capacity;
        CacheEntry* prev_entry = cache->table[remove_index];
        if (prev_entry == to_remove) {
            cache->table[remove_index] = to_remove->hash_next;
        } else {
            while (prev_entry->hash_next != to_remove) {
                prev_entry = prev_entry->hash_next;
            }
            prev_entry->hash_next = to_remove->hash_next;
        }
        free(to_remove);
        cache->size--;
    }
    
    // 创建新的缓存条目
    CacheEntry* new_entry = (CacheEntry*)malloc(sizeof(CacheEntry));
    strncpy(new_entry->domain, domain, sizeof(new_entry->domain) - 1);
    new_entry->domain[sizeof(new_entry->domain) - 1] = '\0';
    strncpy(new_entry->ip, ip, sizeof(new_entry->ip) - 1);
    new_entry->ip[sizeof(new_entry->ip) - 1] = '\0';
    new_entry->prev = NULL;
    new_entry->next = cache->head;
    new_entry->hash_next = cache->table[index];
    
    if (cache->table[index]) {
        cache->table[index]->prev = new_entry;
    }
    cache->table[index] = new_entry;
    
    // 移动到链表头部
    if (cache->head) {
        cache->head->prev = new_entry;
    }
    cache->head = new_entry;
    if (cache->tail == NULL) {
        cache->tail = new_entry;
    }
    
    cache->size++;
}

// 释放缓存
void cache_free() {
    CacheEntry* current = cache->head;
    while (current) {
        CacheEntry* next = current->next;
        free(current);
        current = next;
    }
    free(cache->table);
    free(cache);
}
