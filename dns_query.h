#ifndef DNS_QUERY_H
#define DNS_QUERY_H

void dns_query_init(const char* config_file);
void dns_query_cleanup();
void dns_query_handle_request();
int parse_dns_request(const char* request, char* domain);
int lookup_domain_in_db(const char* domain, char* ip);
void send_dns_response(const char* domain, const char* ip);

#endif // DNS_QUERY_H
