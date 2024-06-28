#include "dns_query.h"
#include "config.h"
#include "log.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    dns_query_init("config.txt");

    while (1) {
        dns_query_handle_request();
    }

    dns_query_cleanup();
    return 0;
}
