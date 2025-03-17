#ifndef SERVER_CALLBACK_H
#define SERVER_CALLBACK_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "define.h"

extern struct board board1;
extern struct board board2;

extern int board1_pausing;
extern int board2_pausing;


void setStatus(char* ip, char* data);
void startDoublePlayer(int sfd);

#endif // SERVER_CALLBACK_H