#ifndef DNS_QUERY_H
#define DNS_QUERY_H

void dns_query_init(const char* upstream_dns_ip);
void dns_query_handle_request(int client_socket);
void dns_query_cleanup();

#endif // DNS_QUERY_H
