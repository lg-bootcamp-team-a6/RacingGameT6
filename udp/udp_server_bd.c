#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <endian.h>  // le32toh, le16toh 사용 (glibc 기반 시스템)

#include "define.h"

#define MAX_BUF 256
#define BOARD_1 192.168.10.3
#define BOARD_2 192.168.10.4

char ip_str[INET_ADDRSTRLEN];
pid_t pid;

void getIPv4Address(const struct sockaddr_in *addr_client, char* buf) {
     if (!addr_client || !buf)
        return;
    
    // 버퍼를 초기화합니다.
    memset(buf, 0, INET_ADDRSTRLEN);
    
    // IPv4 주소를 문자열로 변환하여 buf에 저장합니다.
    if (inet_ntop(AF_INET, &(addr_client->sin_addr), buf, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        buf[0] = '\0'; // 오류 발생 시 빈 문자열 설정
    }
    
}

void handleMessage(char *buf, int len, struct sockaddr_in *addr_client, socklen_t addr_client_len, int sfd) {
    // 최소 메시지 크기: int32_t(4바이트) + int16_t(2바이트)
    if (len < (int)(sizeof(int32_t) + sizeof(int16_t))) {
        fprintf(stderr, "Error: Invalid message size (%d bytes)\n", len);
        return;
    }

    // Little Endian으로 저장된 dest와 cmd를 추출
    //int32_t dest_le;
    int16_t cmd_le;
    //memcpy(&dest_le, buf, sizeof(dest_le));
    memcpy(&cmd_le, buf , sizeof(cmd_le));

    // Little Endian -> 호스트 엔디안 변환
    //int32_t dest = le32toh(dest_le);
    int16_t cmd = le16toh(cmd_le);

    // 데이터 길이 계산
    // int dataSize = len - sizeof(dest_le) - sizeof(cmd_le);
	int dataSize = len - sizeof(cmd_le);
    // 데이터 버퍼 동적 할당 (널 종료 문자 공간 포함)
    char *data = (char *)malloc(dataSize + 1);
    if (data == NULL) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return;
    }
    //memcpy(data, buf + sizeof(dest_le) + sizeof(cmd_le), dataSize);
	memcpy(data, buf + sizeof(cmd_le), dataSize);
    data[dataSize] = '\0';  // 널 종료

	
	getIPv4Address(addr_client, ip_str);

    // 메시지 처리 로직 (여기서는 단순 출력)
    printf("Received message: dest = %s, cmd = %d, data = %s\n", ip_str, cmd, data);

    // 동적 할당 메모리 해제
    free(data);

    // 예제: 받은 메시지를 클라이언트에게 에코 (전송)
    if (sendto(sfd, buf, len, 0, (struct sockaddr *)addr_client, addr_client_len) < 0) {
        perror("sendto failed");
    }
}


int main(int argc, char **argv) {
    int ret;
    int len;
    int sfd;
    struct sockaddr_in addr_server;
    struct sockaddr_in addr_client;
    socklen_t addr_client_len;
    char buf[MAX_BUF];

    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[server]running %s\n", argv[0]);

    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1) {
        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
        return EXIT_FAILURE;
    }

    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_server.sin_port = htons(SERVER_PORT);

    ret = bind(sfd, (struct sockaddr *)&addr_server, sizeof(addr_server));
    if (ret == -1) {
        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
        return EXIT_FAILURE;
    }

    for (;;) {
        addr_client_len = sizeof(addr_client);
        len = recvfrom(sfd, buf, MAX_BUF - 1, 0, (struct sockaddr *)&addr_client, &addr_client_len);
        if (len > 0) {
            buf[len] = 0;
            handleMessage(buf, len, &addr_client, addr_client_len, sfd);
        }
    }

    close(sfd);
    printf("[%d] terminated\n", pid);

    return EXIT_SUCCESS;
}