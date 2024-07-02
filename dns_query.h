#ifndef DNS_QUERY_H
#define DNS_QUERY_H
#include <stdint.h>
#include <stddef.h>
// DNS 报头结构体
typedef struct {
    uint16_t id;      // 事务ID
    uint16_t flags;   // 标志位
    uint16_t qdcount; // 问题数
    uint16_t ancount; // 回答数
    uint16_t nscount; // 授权数
    uint16_t arcount; // 附加数
} DNSHeader;

// DNS 问题部分结构体
typedef struct {
    uint16_t qtype;   // 查询类型
    uint16_t qclass;  // 查询类
} DNSQuestion;

// 设置16位整数的字节顺序（大端）
uint16_t htons_new(uint16_t val);

void dns_query_init(const char* config_file);
void dns_query_cleanup();
void build_dns_query(const char* domain, char* query);
int parse_dns_request(const char* request, char* domain);
int parse_dns_respond(const char* respond, char* ip);
int lookup_domain_in_db(const char* domain, char* ip);
int send_dns_query(const char* query, size_t query_len, char* response, size_t response_len);

void dns_query_handle_request(void* arg);

#endif // DNS_QUERY_H
