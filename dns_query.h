#ifndef DNS_QUERY_H
#define DNS_QUERY_H

void dns_query_init(const char* config_file);
void dns_query_cleanup();
void dns_query_handle_request();

#endif // DNS_QUERY_H
