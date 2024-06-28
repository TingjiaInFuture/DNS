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
