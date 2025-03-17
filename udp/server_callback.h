#ifndef SERVER_CALLBACK_H
#define SERVER_CALLBACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"
#define TOP_N 5
#define MAP_COUNT 4
extern struct board board1;
extern struct board board2;

extern int board1_pausing;
extern int board2_pausing;

typedef struct {
    int scores[TOP_N];  // 내림차순 정렬 (scores[0]이 최고 점수)
    int count;          // 현재 저장된 점수 개수 (최대 TOP_N)
} Ranking;

// 4개의 맵에 대한 랭킹 관리 배열
static Ranking rankingList[MAP_COUNT] = {0};


void setStatus(char* ip, char* data);
void startDoublePlayer(int sfd);
void setMapInfo(ip_str, data);


#endif // SERVER_CALLBACK_H