#include "server_callback.h"
#include <stdio.h>
#include <string.h>
#include "../UdpCmd.h"

// setting start/pause
void setStatus(char *ip, char *data)
{
    if (!strcmp(ip, BOARD_1))
    {
        if (!strcmp(data, "PAUSE"))
        {
            board1.status = STATUS_PAUSE;
            board1_pausing = 1;
        }
        else if (!strcmp(data, "START"))
        {
            board1.status = STATUS_START;
            board1_pausing = 0;
        }
    }
    else if (strcmp(ip, BOARD_2) == 0)
    {
        if (!strcmp(data, "PAUSE"))
        {
            board2.status = STATUS_PAUSE;
            board2_pausing = 1;
        }
        else if (!strcmp(data, "START"))
        {
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
    //char message[] = "start";
    //int msg_len = strlen(message);

    //define messages
    int16_t cmd = GAME_STATUS; // 예시: LOSER에 대한 명령 코드
    char *message_start = "START";

    // 먼저 'cmd'와 'data'를 하나의 버퍼로 결합
    size_t message_size_start = sizeof(cmd) + strlen(message_start) + 1; // cmd + data + NULL terminator

    // Send msg to winner
    char *buffer_start = malloc(message_size_start);
    if (!buffer_start)
    {
        perror("malloc failed for START message\n");
        return;
    }
    memcpy(buffer_start, &cmd, sizeof(cmd));                                      // cmd를 먼저 복사
    memcpy(buffer_start + sizeof(cmd), message_start, strlen(message_start) + 1); // data를 그 뒤에 복사

    if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board1.board_addr, sizeof(board1.board_addr)) < 0)
    {
        perror("failed sendto message for board 1\n");
    }
    else
        printf("Success send message to board 1\n");

    if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board2.board_addr, sizeof(board2.board_addr)) < 0)
    {
        perror("failed sendto message for board 2\n");
    }
    else
        printf("Success send message to board 2\n");
    free(buffer_start); // 메모리 해제
}

void setMapInfo(char *ip_str, char *data)
{
    if (!strcmp(ip_str, BOARD_1))
    {
        board1.map_info = atoi(data);
    }
    else if (!strcmp(ip_str, BOARD_2))
    {
        board2.map_info = atoi(data);
    }
    printf("dest : %s, map info : %d\n", ip_str, atoi(data));
}

void sendIpAddress(char *ip_str, char *data, int sfd)
{
    if (!strcmp(ip_str, BOARD_1))
    {
        //define messages
        int16_t cmd = IP_ADDRESS; // 예시: LOSER에 대한 명령 코드
        char *message_start = board1.ip_address;

        // 먼저 'cmd'와 'data'를 하나의 버퍼로 결합
        size_t message_size_start = sizeof(cmd) + strlen(message_start) + 1; // cmd + data + NULL terminator

        // Send msg to winner
        char *buffer_start = malloc(message_size_start);
        if (!buffer_start)
        {
            perror("malloc failed for START message\n");
            return;
        }
        memcpy(buffer_start, &cmd, sizeof(cmd));                                      // cmd를 먼저 복사
        memcpy(buffer_start + sizeof(cmd), message_start, strlen(message_start) + 1); // data를 그 뒤에 복사

        if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board1.board_addr, sizeof(board1.board_addr)) < 0)
        {
            perror("failed sendto message for board 1\n");
        }
        else
            printf("Success send message to board 1\n");

        free(buffer_start); // 메모리 해제
    }
    else if (!strcmp(ip_str, BOARD_2))
    {
        //define messages
        int16_t cmd = IP_ADDRESS; // 예시: LOSER에 대한 명령 코드
        char *message_start = board2.ip_address;

        // 먼저 'cmd'와 'data'를 하나의 버퍼로 결합
        size_t message_size_start = sizeof(cmd) + strlen(message_start) + 1; // cmd + data + NULL terminator

        // Send msg to winner
        char *buffer_start = malloc(message_size_start);
        if (!buffer_start)
        {
            perror("malloc failed for START message\n");
            return;
        }
        memcpy(buffer_start, &cmd, sizeof(cmd));                           // cmd를 먼저 복사
        memcpy(buffer_start + sizeof(cmd), message_start, strlen(message_start) + 1); // data를 그 뒤에 복사

        if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board2.board_addr, sizeof(board2.board_addr)) < 0)
        {
            perror("failed sendto message for board 2\n");
        }
        else
            printf("Success send message to board 2\n");
        free(buffer_start); // 메모리 해제
    }
}

void addRanking(char *ip_str, char *data)
{
    if (!strcmp(ip_str, BOARD_1))
    {
        update_ranking_for_map(board1.map_info, atof(data));
        print_ranking_for_map(board1.map_info);
    }
    else if (!strcmp(ip_str, BOARD_2))
    {
        update_ranking_for_map(board2.map_info, atof(data));
        print_ranking_for_map(board2.map_info);
    }

}

void update_ranking_for_map(int mapIndex, double newScore)
{
    if (mapIndex < 0 || mapIndex >= MAP_COUNT)
        return;

    Ranking *r = &rankingList[mapIndex];

    // 현재 점수가 하나도 없으면 바로 저장
    if (r->count == 0)
    {
        r->scores[0] = newScore;
        r->count = 1;
        return;
    }

    // 만약 배열이 꽉 찼고 새 점수가 최악의 점수(오름차순에서는 최댓값)보다 크거나 같으면 업데이트하지 않음.
    if (r->count == TOP_N && newScore >= r->scores[TOP_N - 1])
        return;

    // 오름차순 정렬: 낮은 점수가 더 좋으므로, 새 점수가 기존 점수보다 크거나 같으면 뒤로 밀어야 함.
    int pos = 0;
    while (pos < r->count && newScore >= r->scores[pos])
    {
        pos++;
    }

    // 만약 배열이 꽉 찼다면 마지막 원소는 버려지고, 그렇지 않으면 count를 증가
    int limit = (r->count < TOP_N) ? r->count : TOP_N - 1;
    // pos 이후의 요소들을 오른쪽으로 한 칸씩 이동 (최대 TOP_N 개까지)
    for (int i = limit; i > pos; i--)
    {
        r->scores[i] = r->scores[i - 1];
    }

    r->scores[pos] = newScore;
    if (r->count < TOP_N)
    {
        r->count++;
    }

    saveRankingForMap(mapIndex);
}

void saveRankingForMap(int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= MAP_COUNT)
        return;

    Ranking *r = &rankingList[mapIndex];
    char filename[64];
    // 파일 이름은 "mapIndex.txt" 형태로 생성 (예: "0.txt")
    snprintf(filename, sizeof(filename), "%d.txt", mapIndex);

    FILE *fp = fopen(filename, "w");
    if (fp == NULL) {
        perror("fopen failed");
        return;
    }

    // 파일에 랭킹 정보를 저장 (첫 줄에 맵 번호, 이후 각 순위 점수)
    for (int i = 0; i < r->count; i++) {
        fprintf(fp, "%f\n", r->scores[i]);
    }

    fclose(fp);
    printf("Saved ranking for map %d to file %s\n", mapIndex, filename);
}

void print_ranking_for_map(int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= MAP_COUNT)
        return;

    Ranking *r = &rankingList[mapIndex];
    printf("Ranking for map %d:\n", mapIndex);
    for (int i = 0; i < r->count; i++)
    {
        printf("%d: %f\n", i + 1, r->scores[i]);
    }
    printf("\n");
}

void verifyWinner(char *ip_str, char *data, int sfd)
{
    //Check if it is winner
    // Check if ip_str matches board1 or board2's address
    struct sockaddr_in *winner_addr = NULL;
    struct sockaddr_in *loser_addr = NULL;

    // Compare the ip_str with board1.board_addr and board2.board_addr
    if (strcmp(ip_str, inet_ntoa(board1.board_addr.sin_addr)) == 0)
    {
        // ip_str matches board1's address, so board1 is the winner
        winner_addr = &board1.board_addr;
        loser_addr = &board2.board_addr;
    }
    else if (strcmp(ip_str, inet_ntoa(board2.board_addr.sin_addr)) == 0)
    {
        // ip_str matches board2's address, so board2 is the winner
        winner_addr = &board2.board_addr;
        loser_addr = &board1.board_addr;
    }
    else
    {
        // ip_str does not match either board address
        printf("Error: IP address does not match either board1 or board2.\n");
        return;
    }
    printf("Winner is : %s\n", inet_ntoa(winner_addr->sin_addr));

    //send to message to winner : CMD GAME_STATUS, data : WINNER
    //define messages
    int16_t cmd_win = WINNER;  // 예시: LOSER에 대한 명령 코드
    int16_t cmd_loser = LOSER; // 예시: LOSER에 대한 명령 코드
    char *message_win = data;
    char *message_loser = data;

    // 먼저 'cmd'와 'data'를 하나의 버퍼로 결합
    size_t message_size_win = sizeof(cmd_win) + strlen(message_win) + 1;       // cmd + data + NULL terminator
    size_t message_size_loser = sizeof(cmd_loser) + strlen(message_loser) + 1; // cmd + data + NULL terminator

    // Send msg to winner
    char *buffer_win = malloc(message_size_win);
    if (!buffer_win)
    {
        perror("malloc failed for WINNER message\n");
        return;
    }
    memcpy(buffer_win, &cmd_win, sizeof(cmd_win));                              // cmd를 먼저 복사
    memcpy(buffer_win + sizeof(cmd_win), message_win, strlen(message_win) + 1); // data를 그 뒤에 복사

    if (sendto(sfd, buffer_win, message_size_win, 0, (struct sockaddr *)winner_addr, sizeof(*winner_addr)) < 0)
    {
        perror("failed sendto message for WINNER\n");
    }
    printf("Success send message to Winner\n");
    free(buffer_win); // 메모리 해제

    // Sendo msg to loser
    char *buffer_loser = malloc(message_size_loser);
    if (!buffer_loser)
    {
        perror("malloc failed for LOSER message\n");
        return;
    }
    memcpy(buffer_loser, &cmd_loser, sizeof(cmd_loser));                                // cmd를 먼저 복사
    memcpy(buffer_loser + sizeof(cmd_loser), message_loser, strlen(message_loser) + 1); // data를 그 뒤에 복사

    if (sendto(sfd, buffer_loser, message_size_loser, 0, (struct sockaddr *)loser_addr, sizeof(*loser_addr)) < 0)
    {
        perror("failed sendto message for LOSER\n");
    }
    printf("Success send message to LOSER\n");

    free(buffer_loser); // 메모리 해제
}

void updatePosition(char *ip_str, char *data)
{
    float x = 0, y = 0, angle = 0;
    if (sscanf(data, "%f,%f,%f", &x, &y, &angle) == 3)
    {
        //printf("Parsed x = %f, y = %f, angle = %f", x, y, angle);
    }
    else
    {
        perror("Parsing failed!");
    }

    if (!strcmp(ip_str, BOARD_1))
    {
        board1.pos_x = x;
        board1.pos_y = y;
        board1.angle = angle;
    }
    else if (!strcmp(ip_str, BOARD_2))
    {
        board2.pos_x = x;
        board2.pos_y = y;
        board2.angle = angle;
    }
}

void sendRivalPosition(char *ip_str, int sfd)
{
    int16_t cmd = CAR_POSITION;
    char message[100];
    if (!strcmp(ip_str, BOARD_1))
    {
        float x = board2.pos_x;
        float y = board2.pos_y;
        float angle = board2.angle;
        snprintf(message, sizeof(message), "Board 2 : %f,%f,%f\n", x, y, angle);

        size_t message_size_start = sizeof(cmd) + strlen(message) + 1; // cmd + data + NULL terminator

        // Send msg to winner
        char *buffer_start = malloc(message_size_start);
        if (!buffer_start)
        {
            perror("malloc failed for START message\n");
            return;
        }
        memcpy(buffer_start, &cmd, sizeof(cmd));                          // cmd를 먼저 복사
        memcpy(buffer_start + sizeof(cmd), message, strlen(message) + 1); // data를 그 뒤에 복사

        if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board1.board_addr, sizeof(board1.board_addr)) < 0)
        {
            perror("failed sendto message for board 1\n");
        }
        else
            printf("Success send message to board 1\n");
    }
    else if (!strcmp(ip_str, BOARD_2))
    {
        float x = board1.pos_x;
        float y = board1.pos_y;
        float angle = board1.angle;
        snprintf(message, sizeof(message), "Board 1 : %f,%f,%f\n", x, y, angle);

        size_t message_size_start = sizeof(cmd) + strlen(message) + 1; // cmd + data + NULL terminator

        // Send msg to winner
        char *buffer_start = malloc(message_size_start);
        if (!buffer_start)
        {
            perror("malloc failed for START message\n");
            return;
        }
        memcpy(buffer_start, &cmd, sizeof(cmd));                          // cmd를 먼저 복사
        memcpy(buffer_start + sizeof(cmd), message, strlen(message) + 1); // data를 그 뒤에 복사

        if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board2.board_addr, sizeof(board2.board_addr)) < 0)
        {
            perror("failed sendto message for board 2\n");
        }
        else
            printf("Success send message to board 2\n");
    }
}

// share checkpoint for the other
void shareCheckpoint(char *ip, char *data, int sfd)
{
    //define messages
    int16_t cmd = CHECKPOINT;

    // 먼저 'cmd'와 'data'를 하나의 버퍼로 결합
    size_t message_size_checkpoint = sizeof(cmd) + strlen(data) + 1; // cmd + data + NULL terminator

    // Send msg to winner
    char *buffer_checkpoint = malloc(message_size_checkpoint);
    if (!buffer_checkpoint)
    {
        perror("malloc failed for START message\n");
        return;
    }
    memcpy(buffer_checkpoint, &cmd, sizeof(cmd));                    // cmd를 먼저 복사
    memcpy(buffer_checkpoint + sizeof(cmd), data, strlen(data) + 1); // data를 그 뒤에 복사

    if (!strcmp(ip, BOARD_1))
    {
        //share checkpoint to board 2
        if (sendto(sfd, buffer_checkpoint, message_size_checkpoint, 0, (struct sockaddr *)&board2.board_addr, sizeof(board2.board_addr)) < 0)
        {
            perror("failed sendto message for board 2\n");
        }
        else
            printf("Success send message to board 2\n");
    }
    else if (!strcmp(ip, BOARD_2))
    {
        //share checkpoint to board 1
        if (sendto(sfd, buffer_checkpoint, message_size_checkpoint, 0, (struct sockaddr *)&board1.board_addr, sizeof(board1.board_addr)) < 0)
        {
            perror("failed sendto message for board 1\n");
        }
        else
            printf("Success send message to board 1\n");
    }
    printf("changed state\n");
}

void loadRankingForMap(int mapIndex)
{
    if (mapIndex < 0 || mapIndex >= MAP_COUNT)
        return;

    char filename[64];
    snprintf(filename, sizeof(filename), "%d.txt", mapIndex);

    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        perror("fopen failed");
        return;
    }

    Ranking *r = &rankingList[mapIndex];
    r->count = 0;

    char line[256];

    // 이후 각 줄에서 점수를 읽어 최대 TOP_N개만 로딩
    int count = 0;
    while (fgets(line, sizeof(line), fp) != NULL && count < TOP_N) {
        float score;
        if (sscanf(line, "%f", &score) == 1) {
            r->scores[count] = score;
            count++;
        }
    }
    r->count = count;

    fclose(fp);
    printf("Loaded %d scores from file %s for map %d\n", r->count, filename, mapIndex);
}

void sendRanking(char* ip, char* data, int sfd)
{
    int16_t cmd = RANKING;
    char message[100];
    if (!strcmp(ip, BOARD_1))
    {
        int mapIndex = board1.map_info;
        Ranking *r = &rankingList[mapIndex];
        snprintf(message, sizeof(message), "%f,%f,%f,%f,%f\n", r->scores[0], r->scores[1], r-> scores[2], r-> scores[3], r-> scores[4]);

        size_t message_size_start = sizeof(cmd) + strlen(message) + 1; // cmd + data + NULL terminator

        // Send msg to winner
        char *buffer_start = malloc(message_size_start);
        if (!buffer_start)
        {
            perror("malloc failed for START message\n");
            return;
        }
        memcpy(buffer_start, &cmd, sizeof(cmd));                          // cmd를 먼저 복사
        memcpy(buffer_start + sizeof(cmd), message, strlen(message) + 1); // data를 그 뒤에 복사
        printf("ranking data board 1: %s\n", message);
        if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board1.board_addr, sizeof(board1.board_addr)) < 0)
        {
            perror("failed sendto message for board 1\n");
        }
        else
            printf("Success send message to board 1\n");
    }
    else if (!strcmp(ip, BOARD_2))
    {
        int mapIndex = board2.map_info;
        Ranking *r = &rankingList[mapIndex];
        snprintf(message, sizeof(message), "%f,%f,%f,%f,%f\n", r->scores[0], r->scores[1], r-> scores[2], r-> scores[3], r-> scores[4]);

        size_t message_size_start = sizeof(cmd) + strlen(message) + 1; // cmd + data + NULL terminator

        // Send msg to winner
        char *buffer_start = malloc(message_size_start);
        if (!buffer_start)
        {
            perror("malloc failed for START message\n");
            return;
        }
        memcpy(buffer_start, &cmd, sizeof(cmd));                          // cmd를 먼저 복사
        memcpy(buffer_start + sizeof(cmd), message, strlen(message) + 1); // data를 그 뒤에 복사
        printf("ranking data board2: %s\n", message);
        if (sendto(sfd, buffer_start, message_size_start, 0, (struct sockaddr *)&board2.board_addr, sizeof(board2.board_addr)) < 0)
        {
            perror("failed sendto message for board 2\n");
        }
        else
            printf("Success send message to board 2\n");
    }
}