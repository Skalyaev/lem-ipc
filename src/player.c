#include "../include/header.h"

extern t_lemipc data;

byte player_check() {

    const ubyte board_width = data.shm->width / PLAYER_VOLUME;
    const ubyte board_height = data.shm->height / PLAYER_VOLUME;

    const ubyte x = data.self->x > 0 ? data.self->x - 1 : 0;
    const ubyte y = data.self->y > 0 ? data.self->y - 1 : 0;

    t_player* player[MAX_PLAYERS] = {0};
    ushort idx = 0;
    bool down = NO;
    bool found = NO;

    if(semtimedop(data.semid, &data.sem->board_lock,
                  1, &data.sem->timeout)) {

        perror("player_check: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_player* cell;
    for(ubyte xx = 0; xx < 3; xx++) {
        for(ubyte yy = 0; yy < 3; yy++) {

            if(x + xx >= board_width) continue;
            if(y + yy >= board_height) continue;

            cell = &data.shm->board[x + xx][y + yy];
            if(!cell->id) continue;

            if(cell->id == data.self->id) continue;
            if(cell->color == data.self->color) continue;

            found = NO;
            for(ubyte z = 0; z < idx; z++) {

                if(cell->id != player[z]->id) continue;
                found = YES;
                break;
            }
            if(found) continue;
            for(ubyte z = 0; z < idx; z++) {

                if(cell->color != player[z]->color) continue;
                down = YES;
                break;
            }
            if(down) break;
            player[idx++] = cell;
        }
        if(down) break;
    }
    if(semtimedop(data.semid, &data.sem->board_unlock,
                  1, &data.sem->timeout)) {

        perror("player_check: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    return down ? EXIT_GAMEOVER : EXIT_SUCCESS;
}

byte player_think() {

    t_team* team = NULL;
    if(semtimedop(data.semid, &data.sem->teams_lock, 1,
                  &data.sem->timeout)) {

        perror("player_think: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    for(ubyte x = 0; x < data.shm->max_teams; x++) {

        if(!data.shm->teams[x].name[0]) continue;
        if(strcmp(data.shm->teams[x].name, data.opt.team)) continue;
        team = &data.shm->teams[x];
        break;
    }
    if(semtimedop(data.semid, &data.sem->teams_unlock,
                  1, &data.sem->timeout)) {

        perror("player_think: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    const ubyte board_width = data.shm->width / PLAYER_VOLUME;
    const ubyte board_height = data.shm->height / PLAYER_VOLUME;

    t_player board[MAX_WIDTH][MAX_HEIGHT];
    const size_t board_size = sizeof(board);

    if(semtimedop(data.semid, &data.sem->board_lock,
                  1, &data.sem->timeout)) {

        perror("player_think: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    memcpy(board, data.shm->board, board_size);

    if(semtimedop(data.semid, &data.sem->board_unlock,
                  1, &data.sem->timeout)) {

        perror("player_think: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    static const byte directions[5][2] = {
        {0, 0}, {1, 0}, {-1, 0}, {0, 1}, {0, -1}
    };
    data.direction[0] = 0;
    data.direction[1] = 0;

    float min_distance = FLT_MAX;
    float distance;
    bool fled = NO;
    ushort nearest_enemy[2] = {0, 0};

    t_player* player;
    const size_t player_size = sizeof(t_player);

    for(ushort x = 0; x < board_width; x++) {
        for(ushort y = 0; y < board_height; y++) {

            player = &board[x][y];
            if(!player->id || player->id == data.self->id) continue;
            if(player->color == team->color) continue;

            distance = sqrt(pow(data.self->x - x, 2)
                            + pow(data.self->y - y, 2));

            if(distance >= min_distance) continue;
            min_distance = distance;

            memcpy(&data.nearest, player, player_size);
            nearest_enemy[0] = x;
            nearest_enemy[1] = y;
            if(distance < FLED_DISTANCE) fled = YES;
        }
    }
    if(data.target.id) {

        distance = sqrt(pow(data.self->x - data.target.x, 2)
                        + pow(data.self->y - data.target.y, 2));
        if(distance <= min_distance + 2) fled = NO;
    }
    ushort new_x, new_y;
    if(!fled) {
        float best_distance = FLT_MAX;
        for(ubyte i = 0; i < 5; i++) {

            new_x = data.self->x + directions[i][0];
            new_y = data.self->y + directions[i][1];

            if(new_x >= board_width || new_y >= board_height) continue;
            if(directions[i][0] && directions[i][1] && board[new_x][new_y].id) continue;

            distance = sqrt(pow(data.target.x - new_x, 2)
                            + pow(data.target.y - new_y, 2));

            if(distance >= best_distance) continue;
            best_distance = distance;

            data.direction[0] = directions[i][0];
            data.direction[1] = directions[i][1];
        }
        return EXIT_SUCCESS;
    }
    float max_distance = 0.0;
    for(ubyte idx = 0; idx < 5; idx++) {

        new_x = data.self->x + directions[idx][0];
        new_y = data.self->y + directions[idx][1];

        if(new_x >= board_width || new_y >= board_height) continue;
        if(directions[idx][0] && directions[idx][1] && board[new_x][new_y].id)
            continue;

        distance = sqrt(pow(new_x - nearest_enemy[0], 2)
                        + pow(new_y - nearest_enemy[1], 2));

        if(distance <= max_distance) continue;
        max_distance = distance;

        data.direction[0] = directions[idx][0];
        data.direction[1] = directions[idx][1];
    }
    return EXIT_SUCCESS;
}

byte player_communicate() {

    t_team teams[MAX_TEAMS] = {0};
    const size_t team_size = sizeof(teams);

    if(semtimedop(data.semid, &data.sem->teams_lock,
                  1, &data.sem->timeout)) {

        perror("player_communicate: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    printf("player_communicate: locked teams semaphore\n");
    memcpy(teams, data.shm->teams, team_size);

    if(semtimedop(data.semid, &data.sem->teams_unlock,
                  1, &data.sem->timeout)) {

        perror("player_communicate: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    printf("player_communicate: unlocked teams semaphore\n");
    data.target.id = 0;
    t_msg msg = {0};
    sprintf(msg.text, "%d %d", data.nearest.x, data.nearest.y);

    t_team* team = NULL;
    ubyte team_idx = -1;
    ubyte self_idx = -1;
    for(ubyte x = 0; x < data.shm->max_teams; x++) {

        team = &teams[x];
        if(!team->name[0]) continue;
        if(strcmp(team->name, data.opt.team)) continue;
        if(team->players_count < 2) return EXIT_SUCCESS;

        for(ubyte y = 0; y < team->players_count; y++) {

            if(team->ids[y] == data.self->id) self_idx = y;
            msg.type = x + 1 + y;
            if(msgsnd(data.msgid, &msg, MSG_SIZE, IPC_NOWAIT) != -1) continue;
            if(errno == EAGAIN) {

                for(ubyte z = team->players_count; z < MAX_PLAYERS; z++)
                    msgrcv(data.msgid, &msg, MSG_SIZE, x + 1 + z, IPC_NOWAIT);
                continue;
            }
            perror("player_communicate: msgsnd");
            data.code = errno;
            return EXIT_FAILURE;
        }
        team_idx = x;
        break;
    }
    printf("player_communicate: all messages sent\n");
    char positions[MAX_PLAYERS][MSG_SIZE] = {0};
    ubyte count = 0;
    for(ubyte y = 0; y < team->players_count; y++) {

        if(msgrcv(data.msgid, &msg, MSG_SIZE,
                  team_idx + 1 + self_idx, IPC_NOWAIT) == -1) {

            if(errno == ENOMSG) continue;
            perror("player_communicate: msgrcv");
            data.code = errno;
            return EXIT_FAILURE;
        }
        strcpy(positions[y], msg.text);
        count++;
    }
    printf("player_communicate: all messages received\n");
    if(!count) return EXIT_SUCCESS;

    int px[MAX_PLAYERS] = {0};
    int py[MAX_PLAYERS] = {0};
    int cnt[MAX_PLAYERS] = {0};

    int pair_count = 0;
    int max_count = 0;
    int best_index = 0;

    int xx, yy;
    bool found;
    for(ubyte i = 0; i < team->players_count; i++) {

        sscanf(positions[i], "%d %d", &xx, &yy);
        found = NO;
        for(int j = 0; j < pair_count; j++) {

            if(px[j] != xx || py[j] != yy) continue;
            cnt[j]++;
            if(cnt[j] > max_count) {

                max_count = cnt[j];
                best_index = j;
            }
            found = YES;
            break;
        }
        if(found) continue;

        px[pair_count] = xx;
        py[pair_count] = yy;
        cnt[pair_count] = 1;
        if(cnt[pair_count] > max_count) {

            max_count = cnt[pair_count];
            best_index = pair_count;
        }
        pair_count++;
    }
    data.target.id = 1;
    data.target.x = px[best_index];
    data.target.y = py[best_index];
    return EXIT_SUCCESS;
}

byte player_move() {

    if(!data.direction[0] && !data.direction[1]) return EXIT_SUCCESS;

    ubyte new_x = data.self->x + data.direction[0];
    ubyte new_y = data.self->y + data.direction[1];

    if(semtimedop(data.semid, &data.sem->board_lock,
                  1, &data.sem->timeout)) {

        perror("player_move: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_player* dst = &data.shm->board[new_x][new_y];
    if(!dst->id) {

        dst->id = data.self->id;
        dst->color = data.self->color;
        dst->x = new_x;
        dst->y = new_y;

        data.self->id = 0;
        data.self->color = 0;
        data.self = dst;
    }
    if(semtimedop(data.semid, &data.sem->board_unlock,
                  1, &data.sem->timeout)) {

        perror("player_move: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static const char* team_color() {

    if(data.self->color == RGB_RED_) return "\033[31m";
    if(data.self->color == RGB_GREEN_) return "\033[32m";
    if(data.self->color == RGB_YELLOW_) return "\033[33m";
    if(data.self->color == RGB_MAGENTA_) return "\033[35m";
    if(data.self->color == RGB_BLUE_) return "\033[34m";
    if(data.self->color == RGB_CYAN_) return "\033[36m";
    return "";
}

byte player_log() {

    if(semtimedop(data.semid, &data.sem->teams_lock,
                  1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_team* team = NULL;
    for(ubyte x = 0; x < data.shm->max_teams; x++) {

        if(!data.shm->teams[x].name[0]) continue;
        if(strcmp(data.shm->teams[x].name, data.opt.team)) continue;
        team = &data.shm->teams[x];
        break;
    }
    printf("\nPlayer: %d\n", data.self->id);
    printf("Team: %s%s%s\n", team_color(), team->name, RESET);
    printf("Team mates: %d\n", team->players_count);
    printf("Teams count: %d\n", data.shm->teams_count);

    if(semtimedop(data.semid, &data.sem->teams_unlock,
                  1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    if(semtimedop(data.semid, &data.sem->players_count_lock,
                  1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    printf("Players count: %d\n", data.shm->players_count);

    if(semtimedop(data.semid, &data.sem->players_count_unlock,
                  1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    printf("Position: %d, %d\n", data.self->x, data.self->y);
    if(data.target.id) printf("Target: %d, %d\n", data.target.x, data.target.y);
    else printf("Target: none\n");
    return EXIT_SUCCESS;
}
