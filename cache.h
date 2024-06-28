#ifndef CACHE_H
#define CACHE_H

void cache_init(int capacity);
const char* cache_lookup(const char* domain);
void cache_insert(const char* domain, const char* ip);
void cache_free();

#endif // CACHE_H
