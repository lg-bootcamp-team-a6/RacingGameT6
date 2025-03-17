#include "server_callback.h"
#include <stdio.h>
#include <string.h>

// setting start/pause
void setStatus(char* ip, char* data)
{
    if(!strcmp(ip, BOARD_1))
    {
        if(!strcmp(data, "PAUSE")) {
            board1.status = STATUS_PAUSE;
            board1_pausing = 1;
        }
        else if(!strcmp(data, "START")) {
            board1.status = STATUS_START;
            board1_pausing = 0;
        }
    }
    else if(strcmp(ip, BOARD_2) == 0)
    {
        if(!strcmp(data, "PAUSE")) {
            board2.status = STATUS_PAUSE;
            board2_pausing = 1;
        }
        else if(!strcmp(data, "START")) {
            board2.status = STATUS_START;
            board2_pausing = 0;
        }
    }
    printf("changed state\n");
}

void startDoublePlayer(int sfd)
{
    printf("start double player\n");
    sleep(5);
    char message[] = "start";
    int msg_len = strlen(message);
    
    if (sendto(sfd, message, msg_len, 0, (struct sockaddr*)&board1.board_addr, sizeof(board1.board_addr)) < 0) {
        perror("sendto board1 failed");
    }
    if (sendto(sfd, message, msg_len, 0, (struct sockaddr*)&board2.board_addr, sizeof(board2.board_addr)) < 0) {
        perror("sendto board2 failed");
    }
    printf("successfully sended.\n");
}
