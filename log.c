// log.c
#define _CRT_SECURE_NO_WARNINGS

#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static FILE* log_file = NULL;

void log_init() {
    log_file = fopen("dnsrelay.log", "a");
    if (log_file == NULL) {
        log_error("Failed to open log file");
        log_file = stderr;  // fallback to stderr
    }
}

void log_close() {
    if (log_file && log_file != stderr) {
        fclose(log_file);
    }
}

void log_debug(const char* format, ...) {
    if (log_file == NULL) return;

    // 获取当前时间并格式化
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buffer[20]; // 足够存储时间字符串
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, format);

    // 在日志消息前添加时间戳
    fprintf(log_file, "%s DEBUG: ", time_buffer);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    va_end(args);
}

void log_error(const char* format, ...) {
    if (log_file == NULL) return;

    // 获取当前时间并格式化
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buffer[20]; // 足够存储时间字符串
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, format);

    // 在日志消息前添加时间戳
    fprintf(log_file, "%s ERROR: ", time_buffer);
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    va_end(args);
}
void console_log_basic(const char* client_ip, const char* query_domain, int sequence_number) {
    time_t now = time(NULL);
    char* timestamp = ctime(&now);
    timestamp[strlen(timestamp) - 1] = '\0'; // Remove newline character
    printf("[%s] %d %s %s\n", timestamp, sequence_number, client_ip, query_domain);
}
void console_log_detail(const char* format, ...) {

    // 获取当前时间并格式化
    time_t now = time(NULL);
    struct tm* tm_info = localtime(&now);
    char time_buffer[20]; // 足够存储时间字符串
    strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", tm_info);

    va_list args;
    va_start(args, format);

    fprintf(stdout, "%s : ", time_buffer);
    vfprintf(stdout, format, args);
    fprintf(stdout, "\n");

    va_end(args);
}