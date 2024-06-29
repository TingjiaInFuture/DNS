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
int send_dns_query(SOCKET sockfd, const struct sockaddr_in* servaddr, const char* request);
int receive_dns_response(SOCKET sockfd, char* response);

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
