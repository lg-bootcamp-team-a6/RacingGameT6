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

    print_ranking_for_map(0);
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
        printf("%d: %d\n", i + 1, r->scores[i]);
    }
    printf("\n");
}
