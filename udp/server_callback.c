#include "server_callback.h"
#include <stdio.h>
#include <string.h>
#include "../UdpCmd.h"

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

void setMapInfo(char* ip_str, char* data)
{
    if(!strcmp(ip_str, BOARD_1))
    {
        board1.map_info = atoi(data);
    }
    else if(!strcmp(ip_str, BOARD_2))
    {
        board2.map_info = atoi(data);
    }
    printf("dest : %s, map info : %d\n", ip_str, atoi(data));
}

void addRanking(char* ip_str, char* data)
{
     if(!strcmp(ip_str, BOARD_1))
    {
        update_ranking_for_map(board1.map_info, atof(data));
    }
    else if(!strcmp(ip_str, BOARD_2))
    {
        update_ranking_for_map(board2.map_info, atof(data));
    }

    print_ranking_for_map(3);
    // print_ranking_for_map(1);
    // print_ranking_for_map(2);
    // print_ranking_for_map(3);
}

void update_ranking_for_map(int mapIndex, double newScore) {
    if (mapIndex < 0 || mapIndex >= MAP_COUNT)
        return;
    
    Ranking *r = &rankingList[mapIndex];

    // 현재 점수가 하나도 없으면 바로 저장
    if (r->count == 0) {
        r->scores[0] = newScore;
        r->count = 1;
        return;
    }

    // 만약 배열이 꽉 찼고 새 점수가 최하위보다 작거나 같으면 업데이트하지 않음
    if (r->count == TOP_N && newScore <= r->scores[TOP_N - 1])
        return;

    // 내림차순 배열에서 새 점수를 삽입할 위치 찾기 (최대 5회 비교)
    int pos = 0;
    while (pos < r->count && newScore <= r->scores[pos]) {
        pos++;
    }
    
    // 꽉 찼다면 마지막 원소는 버려지고, 그렇지 않으면 count를 증가
    int limit = (r->count < TOP_N) ? r->count : TOP_N - 1;
    // pos 이후의 요소들을 오른쪽으로 한 칸씩 이동 (최대 5회 이동)
    for (int i = limit; i > pos; i--) {
        r->scores[i] = r->scores[i - 1];
    }
    
    r->scores[pos] = newScore;
    if (r->count < TOP_N) {
        r->count++;
    }
}

void print_ranking_for_map(int mapIndex) {
    if (mapIndex < 0 || mapIndex >= MAP_COUNT)
        return;
    
    Ranking *r = &rankingList[mapIndex];
    printf("Ranking for map %d:\n", mapIndex);
    for (int i = 0; i < r->count; i++) {
        printf("%d: %f\n", i + 1, r->scores[i]);
    }
    printf("\n");
}

void verifyWinner(char* ip_str, char* data, int sfd)
{
    //Check if it is winner
    // Check if ip_str matches board1 or board2's address
    struct sockaddr_in *winner_addr = NULL;
    struct sockaddr_in *loser_addr = NULL;

    // Compare the ip_str with board1.board_addr and board2.board_addr
    if (strcmp(ip_str, inet_ntoa(board1.board_addr.sin_addr)) == 0) {
        // ip_str matches board1's address, so board1 is the winner
        winner_addr = &board1.board_addr;
        loser_addr = &board2.board_addr;
    } else if (strcmp(ip_str, inet_ntoa(board2.board_addr.sin_addr)) == 0) {
        // ip_str matches board2's address, so board2 is the winner
        winner_addr = &board2.board_addr;
        loser_addr = &board1.board_addr;
    } else {
        // ip_str does not match either board address
        printf("Error: IP address does not match either board1 or board2.\n");
        return;
    }
    printf("Winner is : %s\n", inet_ntoa(winner_addr->sin_addr));

    //send to message to winner : CMD GAME_STATUS, data : WINNER
    //define messages
    int16_t cmd = GAME_STATUS; // 예시: LOSER에 대한 명령 코드
    char *message_win = "WINNER";
    char *message_loser = "LOSER";

    // 먼저 'cmd'와 'data'를 하나의 버퍼로 결합
    size_t message_size_win = sizeof(cmd) + strlen(message_win) + 1;  // cmd + data + NULL terminator
    size_t message_size_loser = sizeof(cmd) + strlen(message_loser) + 1;  // cmd + data + NULL terminator

    // Send msg to winner
    char *buffer_win = malloc(message_size_win);
    if (!buffer_win) {
        perror("malloc failed for WINNER message\n");
        return;
    }
    memcpy(buffer_win, &cmd, sizeof(cmd));  // cmd를 먼저 복사
    memcpy(buffer_win + sizeof(cmd), message_win, strlen(message_win) + 1);  // data를 그 뒤에 복사

    if (sendto(sfd, buffer_win, message_size_win, 0, (struct sockaddr*)winner_addr, sizeof(*winner_addr)) < 0) {
        perror("failed sendto message for WINNER\n");
    }
    printf("Success send message to Winner\n");
    free(buffer_win);  // 메모리 해제

    // Sendo msg to loser
    char *buffer_loser = malloc(message_size_loser);
    if (!buffer_loser) {
        perror("malloc failed for LOSER message\n");
        return;
    }
    memcpy(buffer_loser, &cmd, sizeof(cmd));  // cmd를 먼저 복사
    memcpy(buffer_loser + sizeof(cmd), message_loser, strlen(message_loser) + 1);  // data를 그 뒤에 복사

    if (sendto(sfd, buffer_loser, message_size_loser, 0, (struct sockaddr*)loser_addr, sizeof(*loser_addr)) < 0) {
        perror("failed sendto message for LOSER\n");
    }
    printf("Success send message to LOSER\n");

    free(buffer_loser);  // 메모리 해제
    }
