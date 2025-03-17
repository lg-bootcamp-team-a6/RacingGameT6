#ifndef DEFINE_H
#define DEFINE_H

#include <stdint.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <endian.h>  // le32toh, le16toh 사용 (glibc 기반 시스템)
#include "../UdpCmd.h"
#include "server_callback.h"
#include <stdbool.h> 


// 함수 프로토타입 정의
void getIPv4Address(const struct sockaddr_in *addr_client, char* buf);
void handleMessage(char *buf, int len, struct sockaddr_in *addr_client, socklen_t addr_client_len, int sfd);
void statusRegister();

#endif // DEFINE_H