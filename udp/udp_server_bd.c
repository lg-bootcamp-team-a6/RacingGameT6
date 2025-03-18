#include "udp_server_bd.h"

#define MAX_BUF 256

char ip_str[INET_ADDRSTRLEN];
pid_t pid;

struct board board1;
struct board board2;
int playMode = 0;
bool is_winner = 0;
int double_played = 0;
int board1_pausing = 0;
int board2_pausing = 0;
int finished = 0;
// 0 : single player, 1 : double player

// 보드 정보 초기화 (소켓 오류 시 close() 호출 필요 없음)
void init_board() {
    // board1 설정
    board1.ip_address = BOARD_1;
    board1.checkpointIndex = -1;
    board1.status = -1;
    board1.time = "";
	board1.map_info = -1;
    board1.pos_x = -1;
    board1.pos_y = -1;
    board1.angle = 0;
    memset(&board1.board_addr, 0, sizeof(board1.board_addr));
    board1.board_addr.sin_family = AF_INET;
    board1.board_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, board1.ip_address, &board1.board_addr.sin_addr) <= 0) {
        perror("Invalid address for board1");
        exit(EXIT_FAILURE);
    }

    // board2 설정
    board2.ip_address = BOARD_2;
    board2.checkpointIndex = -1;
    board2.status = -1;
    board2.time = "";
	board2.map_info = -1;
    board2.pos_x = -1;
    board2.pos_y = -1;
    board2.angle = 0;
    memset(&board2.board_addr, 0, sizeof(board2.board_addr));
    board2.board_addr.sin_family = AF_INET;
    board2.board_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, board2.ip_address, &board2.board_addr.sin_addr) <= 0) {
        perror("Invalid address for board2");
        exit(EXIT_FAILURE);
    }
}

void getIPv4Address(const struct sockaddr_in *addr_client, char* buf) {
    if (!addr_client || !buf)
        return;
    
    memset(buf, 0, INET_ADDRSTRLEN);
    if (inet_ntop(AF_INET, &(addr_client->sin_addr), buf, INET_ADDRSTRLEN) == NULL) {
        perror("inet_ntop");
        buf[0] = '\0';
    }
}

//board to server
void handleMessage(char *buf, int len, struct sockaddr_in *addr_client, socklen_t addr_client_len, int sfd) {
    if (len < (int)(sizeof(int16_t))) {
        fprintf(stderr, "Error: Invalid message size (%d bytes)\n", len);
        return;
    }
    
    int16_t cmd_le;
    memcpy(&cmd_le, buf, sizeof(cmd_le));
    int16_t cmd = le16toh(cmd_le);
    
    int dataSize = len - sizeof(cmd_le);
    char *data = malloc(dataSize + 1);
    if (!data) {
        fprintf(stderr, "Error: Memory allocation failed.\n");
        return;
    }
    memcpy(data, buf + sizeof(cmd_le), dataSize);
    data[dataSize] = '\0';
    
    getIPv4Address(addr_client, ip_str);
    //printf("Received message: dest = %s, cmd = %d, data = %s\n", ip_str, cmd, data);
    //printf("playMode : %d, is_winner : %d\n", playMode, is_winner);

    switch(cmd)
    {
        case GAME_STATUS:
			printf("case 0 : start or pause\n");
            setStatus(ip_str, data);
            if (!playMode && board1_pausing && board2_pausing) {
				printf("double\n");
                playMode = 1;
                if(board1.map_info == board2.map_info) startDoublePlayer(sfd);
            }
            break;
        case CHECKPOINT:
            //printf("case 1 : Get checkpoint %s\n",data);
            if(playMode) shareCheckpoint(ip_str, data,sfd);

            break;
        case CAR_POSITION:
            updatePosition(ip_str, data);
            sendRivalPosition(ip_str, sfd);
            break;
		//finish
        case FINISH:
			printf("case 3: finish map\n");
			addRanking(ip_str, data);
            finished = 1;
            //printf("playMode : %d, is_winner : %d\n", playMode, is_winner);
            if (playMode && !is_winner){
                printf("who is the winner : %s\n", ip_str);
                verifyWinner(ip_str, data, sfd);
                is_winner = 1;
            }
            playMode = 0;
            break;
		//which map
		case MAP_STATUS:
			printf("change map\n");
			setMapInfo(ip_str, data);
            playMode = 0;
            //reset winner information
            is_winner = 0;
            sendIpAddress(ip_str, data, sfd);
            sendRanking(ip_str, data, sfd);
			break;
        default:
            break;
    }
    
    free(data);
    
}

int main(int argc, char **argv) {
    int ret, len, sfd;
    struct sockaddr_in addr_server, addr_client;
    socklen_t addr_client_len;
    char buf[MAX_BUF];
    
    if (argc != 1) {
        printf("usage: %s\n", argv[0]);
        return EXIT_FAILURE;
    }
    
    printf("[server] running %s\n", argv[0]);
    
    sfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sfd == -1) {
        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
        return EXIT_FAILURE;
    }
    
    init_board();
    
    memset(&addr_server, 0, sizeof(addr_server));
    addr_server.sin_family = AF_INET;
    addr_server.sin_addr.s_addr = htonl(INADDR_ANY);
    addr_server.sin_port = htons(SERVER_PORT);
    
    ret = bind(sfd, (struct sockaddr *)&addr_server, sizeof(addr_server));
    if (ret == -1) {
        printf("[%d] error: %s (%d)\n", pid, strerror(errno), __LINE__);
        close(sfd);
        return EXIT_FAILURE;
    }
    loadRankingForMap(0);
    loadRankingForMap(1);
    loadRankingForMap(2);
    loadRankingForMap(3);
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
