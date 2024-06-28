// log.c
#include "log.h"
#include <stdio.h>
#include <stdarg.h>
#include <time.h>

static FILE *log_file = NULL;

void log_init() {
    log_file = fopen("dnsrelay.log", "a");
    if (log_file == NULL) {
        perror("Failed to open log file");
        log_file = stderr;  // fallback to stderr
    }
}

void log_close() {
    if (log_file && log_file != stderr) {
        fclose(log_file);
    }
}

void log_debug(const char *format, ...) {
    if (log_file == NULL) return;

    va_list args;
    va_start(args, format);

    fprintf(log_file, "DEBUG: ");
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    va_end(args);
}

void log_error(const char *format, ...) {
    if (log_file == NULL) return;

    va_list args;
    va_start(args, format);

    fprintf(log_file, "ERROR: ");
    vfprintf(log_file, format, args);
    fprintf(log_file, "\n");

    va_end(args);
}
