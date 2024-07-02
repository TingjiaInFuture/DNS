// log.h
#ifndef LOG_H
#define LOG_H

void log_init();
void log_close();
void log_debug(const char* format, ...);
void log_error(const char* format, ...);
void console_log_basic(const char* client_ip, const char* query_domain, int sequence_number);
void console_log_detail(const char* format, ...);
#endif // LOG_H
