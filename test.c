/**
 * Alert: This is a test file!
 * comment out the main function before running the DNS.
 */

// main.c (测试用例)
#include "cache.h"
#include <stdio.h>

int main() {
    cache_init(3);

    cache_insert("example.com", "1.2.3.4");
    cache_insert("example.org", "2.3.4.5");
    cache_insert("example.net", "3.4.5.6");

    const char* ip = cache_lookup("example.com");
    if (ip) {
        printf("example.com: %s\n", ip);
    } else {
        printf("example.com not found\n");
    }

    cache_insert("example.edu", "4.5.6.7");

    ip = cache_lookup("example.org");
    if (ip) {
        printf("example.org: %s\n", ip);
    } else {
        printf("example.org not found\n");
    }

    ip = cache_lookup("example.net");
    if (ip) {
        printf("example.net: %s\n", ip);
    } else {
        printf("example.net not found\n");
    }

    ip = cache_lookup("example.edu");
    if (ip) {
        printf("example.edu: %s\n", ip);
    } else {
        printf("example.edu not found\n");
    }

    cache_free();
    return 0;
}

// // main.c (测试用例)
// #include "log.h"

// int main() {
//     log_init();
    
//     log_debug("This is a debug message.");
//     log_error("This is an error message.");
    
//     log_close();
//     return 0;
// }
