#ifndef PLATFORM_H
#define PLATFORM_H

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

unsigned socket_init();
void socket_cleanup();
SOCKET create_udp_socket();
struct sockaddr_in create_server_address(const char* ip, int port);
void build_dns_query(const char* domain, char* query, int* query_len);
int send_dns_query(SOCKET sockfd, const struct sockaddr_in* servaddr, const char* request, int request_len);
int receive_dns_response(SOCKET sockfd, struct in_addr* response_ip);

// DNS报文头部
typedef struct {
    unsigned short id;      // 标识符
    unsigned short flags;   // 标志字段
    unsigned short qdcount; // 问题数目
    unsigned short ancount; // 回答资源记录数目
    unsigned short nscount; // 授权资源记录数目
    unsigned short arcount; // 附加资源记录数目
} dns_header_t;

// DNS问题部分
typedef struct {
    char qname[256];        // 查询名
    unsigned short qtype;   // 查询类型
    unsigned short qclass;  // 查询类
} dns_question_t;

// DNS资源记录
typedef struct {
    char* name;             // 域名
    unsigned short type;    // 类型
    unsigned short class;   // 类
    unsigned int ttl;       // 生存时间
    unsigned short rdlength;// 资源数据长度
    char* rdata;            // 资源数据
} dns_resource_record_t;

#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#define closesocket close
#define SOCKET int
#define SOCKET_ERROR -1
#define INVALID_SOCKET -1

#define socket_init() // 在 Linux 上，这个宏为空
#define socket_cleanup()
#define create_udp_socket()
#define create_server_address()
#define send_dns_query()
#define receive_dns_response()
#endif

#endif // PLATFORM_H
