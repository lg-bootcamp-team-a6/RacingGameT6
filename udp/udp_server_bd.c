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

#include "define.h"

#define MAX_BUF 256

pid_t pid;

int main(int argc, char **argv)
{
	int ret;
	int len;
	int sfd;
	struct sockaddr_in addr_server;
	struct sockaddr_in addr_client;
	socklen_t addr_client_len;
	char buf[MAX_BUF];

	if(argc != 1) {
		printf("usage: %s\n", argv[0]);
		return EXIT_FAILURE;
	}
	printf("[server]running %s\n", argv[0]);

	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sfd == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	memset(&addr_server, 0, sizeof(addr_server));
	addr_server.sin_family = AF_INET;
	addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
	addr_server.sin_port = htons(SERVER_PORT);
	ret = bind(sfd, (struct sockaddr *)&addr_server, sizeof(addr_server));
	if(ret == -1) {
		printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
		return EXIT_FAILURE;
	}

	int s32LineCount = 0;
	for(;;) {
		addr_client_len = sizeof(addr_client);
		len = recvfrom(sfd, buf, MAX_BUF-1, 0, (struct sockaddr *)&addr_client, &addr_client_len);
		if(len > 0) {
			buf[len] = 0;
			printf("[%d] received: %s\n", pid, buf);

			FILE *file = fopen(FILE_NAME, "a");  // 파일을 append 모드로 열기
            if (file) {
                fprintf(file, "data[%d]: %s\n", s32LineCount, buf);
                fclose(file);
				printf("data is appended to file; \"data[%d]: %s\n\"", s32LineCount, buf);
				s32LineCount++;
            } else {
                printf("[%d] error: Failed to open file (%d)\n", pid, __LINE__);
            }

			/* echo back */
			// sendto(sfd, buf, len, 0,  (struct sockaddr *)&addr_client, sizeof(addr_client));
		}
	}

	close(sfd);

	printf("[%d] terminated\n", pid);

	return EXIT_SUCCESS;
}

