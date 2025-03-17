#pragma once
#define SERVER_PORT 12345
#define FILE_NAME "data.txt"
#include <unistd.h>     // sleep 함수 사용
#include <arpa/inet.h>  // inet_pton
#include <sys/socket.h> // socket 관련 함수
#include <string.h>     // memset, strlen
#include <stdio.h>      // perror

#define BOARD_1 "192.168.10.3"
#define BOARD_2 "192.168.10.4"
#define STATUS_PAUSE 1
#define STATUS_START 0

struct board{
    char* ip_address;
    int checkpointIndex;
    int status;
    char* time;
    int map_info;
    int pos_x;
    int pos_y;
    struct sockaddr_in board_addr;
};