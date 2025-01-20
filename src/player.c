#include "../include/header.h"

extern t_lemipc data;

byte player_check() {

    const ushort posX = data.self.x;
    const ushort posY = data.self.y;

    t_player* players[MAX_PLAYERS] = {0};
    t_player* player;

    ubyte idx = 0;
    bool gameover = NO;
    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_check: semtimedop");
        return EXIT_FAILURE;
    }
    for(ushort x = posX - 1; x < posX + 2; x++) {
        for(ushort y = posY - 1; y < posY + 2; y++) {

            if(x >= BOARD_WIDTH || y >= BOARD_HEIGHT) continue;
            if(x == posX && y == posY) continue;

            player = &data.shm->board[x][y];
            if(!player->id) continue;
            if(player->color == data.self.color) continue;

            for(ubyte z = 0; z < idx; z++) {

                if(players[z]->color != player->color) continue;
                gameover = YES;
                break;
            }
            if(gameover) break;
            players[idx++] = player;
        }
        if(gameover) break;
    }
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_check: semtimedop");
        return EXIT_FAILURE;
    }
    return gameover ? EXIT_GAMEOVER : EXIT_SUCCESS;
}

static int direction_cmp_target(const void* a, const void* b) {

    const byte* d1 = a;
    const byte* d2 = b;
    double dist1 = sqrt(pow(data.self.x + d1[0] - data.target.x, 2)
                        + pow(data.self.y + d1[1] - data.target.y, 2));
    double dist2 = sqrt(pow(data.self.x + d2[0] - data.target.x, 2)
                        + pow(data.self.y + d2[1] - data.target.y, 2));
    return (dist1 > dist2) - (dist1 < dist2);
}

static int direction_cmp_nearest(const void* a, const void* b) {

    const byte* d1 = a;
    const byte* d2 = b;
    double dist1 = sqrt(pow(data.self.x + d1[0] - data.nearest.x, 2)
                        + pow(data.self.y + d1[1] - data.nearest.y, 2));
    double dist2 = sqrt(pow(data.self.x + d2[0] - data.nearest.x, 2)
                        + pow(data.self.y + d2[1] - data.nearest.y, 2));
    return (dist1 < dist2) - (dist1 > dist2);
}

byte player_think() {

    static const byte directions[4][2] = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    static t_player board[BOARD_WIDTH][BOARD_HEIGHT];

    static const size_t board_size = sizeof(board);
    static const size_t directions_size = sizeof(directions);
    static const size_t diration_size = sizeof(byte) * 2;

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_think: semtimedop");
        return EXIT_FAILURE;
    }
    memcpy(board, data.shm->board, board_size);
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_think: semtimedop");
        return EXIT_FAILURE;
    }
    const ushort posX = data.self.x;
    const ushort posY = data.self.y;
    memset(&data.nearest, 0, PLAYER_SIZE);

    ushort minDistance = USHRT_MAX;
    ushort x, y, distance;
    t_player* player;

    for(x = 0; x < BOARD_WIDTH; x++) {
        for(y = 0; y < BOARD_HEIGHT; y++) {

            player = &board[x][y];
            if(!player->id) continue;
            if(player->color == data.self.color) continue;

            distance = (x - posX) * (x - posX) + (y - posY) * (y - posY);
            if(distance >= minDistance) continue;

            minDistance = distance;
            memcpy(&data.nearest, player, PLAYER_SIZE);
        }
    }
    memcpy(data.self.directions, directions, directions_size);

    if(data.target.id) {
        qsort(data.self.directions, 4, diration_size, direction_cmp_target);
    } else if(minDistance < 100)
        qsort(data.self.directions, 4, diration_size, direction_cmp_nearest);
    else
        data.self.directions[0][0] = data.self.directions[0][1] = 0;
    return EXIT_SUCCESS;
}

byte player_communicate() {

    t_team team = {0};
    ubyte myidx = 0;
    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_communicate: semtimedop");
        return EXIT_FAILURE;
    }
    for(ubyte x = 0; x < MAX_TEAMS; x++) {

        if(!data.shm->teams[x].players_count) continue;
        if(data.self.color != data.shm->teams[x].color) continue;

        memcpy(&team, &data.shm->teams[x], TEAM_SIZE);
        break;
    }
    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_communicate: semtimedop");
        return EXIT_FAILURE;
    }
    for(ubyte x = 0; x < team.players_count && !myidx; x++)
        if(team.ids[x] == data.self.id) myidx = x + 1;

    data.target.id = 0;
    if(!myidx) return EXIT_FAILURE;

    t_msg recv = {0};
    t_msg send = {0};
    sprintf(send.text, "%d, %d %d", myidx, data.nearest.x, data.nearest.y);

    for(ubyte x = 0; x < team.players_count; x++) {

        send.type = team.ids[x];
        if(msgsnd(team.msgid, &send, IPC_MSG_SIZE, IPC_NOWAIT) != -1) continue;
        if(errno == EAGAIN) {

            for(ubyte y = team.players_count; y < MAX_PLAYERS; y++)
                msgrcv(team.msgid, &recv, IPC_MSG_SIZE, team.ids[y], IPC_NOWAIT);
            continue;
        }
        data.code = errno;
        perror("player_communicate: msgsnd");
        return EXIT_FAILURE;
    }
    char msgs[team.players_count][IPC_MSG_SIZE];
    memset(msgs, 0, sizeof(msgs));

    ubyte count = 0;
    for(ubyte x = 0; x < team.players_count; x++) {

        if(msgrcv(team.msgid, &recv, IPC_MSG_SIZE, data.self.id, IPC_NOWAIT) == -1) {

            if(errno == ENOMSG) continue;
            data.code = errno;
            perror("player_communicate: msgrcv");
            return EXIT_FAILURE;
        }
        strcpy(msgs[x], recv.text);
        count++;
    }
    if(count < 2) return EXIT_SUCCESS;

    t_position positions[team.players_count];
    memset(positions, 0, sizeof(positions));

    ubyte idx;
    ushort x, y;
    for(count = 0; count < team.players_count; count++) {

        sscanf(msgs[count], "%hhu, %hu %hu", &idx, &x, &y);
        if(idx > team.players_count) continue;

        positions[idx - 1].x = x;
        positions[idx - 1].y = y;
        positions[idx - 1].set = YES;
    }
    ubyte best_idx = 0;
    ubyte best_count = 0;
    for(ubyte x = 0; x < team.players_count; x++) {

        if(!positions[x].set) continue;
        count = 0;
        for(ubyte y = 0; y < team.players_count; y++)
            if(x != y && positions[y].set
                    && positions[x].x == positions[y].x
                    && positions[x].y == positions[y].y) count++;

        if(count < best_count) continue;
        best_idx = x;
        best_count = count;
    }
    data.target.x = positions[best_idx].x;
    data.target.y = positions[best_idx].y;
    data.target.id = 1;
    return EXIT_SUCCESS;
}

byte player_move() {

    const ushort posX = data.self.x;
    const ushort posY = data.self.y;

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_move: semtimedop");
        return EXIT_FAILURE;
    }
    t_player* player;
    ushort newX, newY;
    for(ubyte x = 0; x < 4; x++) {

        newX = posX + data.self.directions[x][0];
        newY = posY + data.self.directions[x][1];
        if(newX >= BOARD_WIDTH || newY >= BOARD_HEIGHT) continue;

        player = &data.shm->board[newX][newY];
        if(player->id == data.self.id) break;
        if(player->id) continue;

        player->id = data.self.id;
        player->color = data.self.color;

        player = &data.shm->board[posX][posY];
        player->id = 0;
        player->color = 0;

        data.self.x = newX;
        data.self.y = newY;
        break;
    }
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_move: semtimedop");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static const char* team_color() {

    if(data.self.color == RGB_RED_) return RED;
    if(data.self.color == RGB_GREEN_) return GREEN;
    if(data.self.color == RGB_YELLOW_) return YELLOW;
    if(data.self.color == RGB_MAGENTA_) return MAGENTA;
    if(data.self.color == RGB_BLUE_) return BLUE;
    if(data.self.color == RGB_CYAN_) return CYAN;
    return "";
}

byte player_log() {

    t_team myteam = {0};
    t_team* team = NULL;
    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_log: semtimedop");
        return EXIT_FAILURE;
    }
    for(ubyte x = 0; x < MAX_TEAMS; x++) {

        team = &data.shm->teams[x];
        if(!team->players_count) continue;
        if(team->color != data.self.color) continue;
        memcpy(&myteam, team, TEAM_SIZE);
        break;
    }
    const ubyte teams_count = data.shm->teams_count;
    const ubyte players_count = data.shm->players_count;

    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("player_log: semtimedop");
        return EXIT_FAILURE;
    }
    printf("\nPlayer: %d\n", data.self.id);
    printf("Position: %d, %d\n", data.self.x, data.self.y);
    printf("Team: %s%s%s\n", team_color(), myteam.name, RESET);
    printf("Team mates: %d\n", myteam.players_count - 1);
    printf("Teams count: %d\n", teams_count);
    printf("Players count: %d\n", players_count);

    if(data.target.id) printf("Target: %d, %d\n", data.target.x, data.target.y);
    else printf("Target: none\n");
    if(data.nearest.id) printf("Nearest enemy: %d, %d\n", data.nearest.x, data.nearest.y);
    else printf("Nearest enemy: none\n");
    return EXIT_SUCCESS;
}
