// log.h
#ifndef LOG_H
#define LOG_H

void log_init();
void log_close();
void log_debug(const char *format, ...);
void log_error(const char *format, ...);

#endif // LOG_H
