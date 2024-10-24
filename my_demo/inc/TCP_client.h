#ifndef __TCP_CLIENT_H // 定义以防止递归包含
#define __TCP_CLIENT_H

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>

extern int tcp_socket;

extern int tcp_client_init();

#endif