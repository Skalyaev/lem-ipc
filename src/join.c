#include "../include/header.h"

extern t_lemipc data;

static byte failure(const char* const msg) {

    if(data.first) data.abort = YES;
    data.code = errno;
    perror(msg);
    return EXIT_FAILURE;
}

static t_team* join_team() {

    if(data.shm->players_count == data.shm->max_players) return NULL;
    t_team* team = NULL;
    ushort x;
    for(x = 0; x < data.shm->max_teams; x++) {

        if(x >= data.shm->teams_count || !data.shm->teams[x].name[0]) {

            strcpy(data.shm->teams[x].name, data.opt.team);
            team = &data.shm->teams[x];
            data.shm->teams_count++;
            break;
        }
        if(strcmp(data.opt.team, data.shm->teams[x].name)) continue;
        team = &data.shm->teams[x];
        break;
    }
    return team;
}

static bool try_cell(ushort* const w, ushort* const x, t_team* const team) {

    if(data.shm->board[*w][*x].team) return NO;

    for(ushort y = 0; y < PLAYER_VOLUME; y++)
        for(ushort z = 0; z < PLAYER_VOLUME; z++)
            if(data.shm->board[*w + y][*x + z].team) return NO;

    t_player* player;
    for(ushort y = 0; y < PLAYER_VOLUME; y++) {
        for(ushort z = 0; z < PLAYER_VOLUME; z++) {

            player = &data.shm->board[*w + y][*x + z];
            player->x = *w;
            player->y = *x;
            player->team = team;
        }
    }
    data.self = player;
    data.self->team->players_count++;
    data.shm->players_count++;
    data.joined = YES;
    return YES;
}

static void join_board(t_team* const team) {

    srand(time(NULL));
    ushort x = rand() % data.shm->width;
    ushort y = rand() % data.shm->height;
    if(x > data.shm->width - PLAYER_VOLUME) x = 0;
    if(y > data.shm->height - PLAYER_VOLUME) y = 0;

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {
        perror("semtimedop");
        return;
    }
    while(YES) {
        if(try_cell(&x, &y, team)) break;
        x += PLAYER_VOLUME;
        if(x <= data.shm->width - PLAYER_VOLUME) continue;

        x = 0;
        y += PLAYER_VOLUME;
        if(y > data.shm->height - PLAYER_VOLUME) y = 0;
    }
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {
        perror("semtimedop");
        return;
    }
}

byte join() {

    if(semtimedop(data.semid, &data.sem->init_lock, 1, &data.sem->timeout))
        return failure("semtimedop");

    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        if(semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout))
            perror("semtimedop");
        return failure("semtimedop");
    }
    if(semtimedop(data.semid, &data.sem->players_count_lock, 1, &data.sem->timeout)) {

        if(semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout))
            perror("semtimedop");
        if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout))
            perror("semtimedop");
        return failure("semtimedop");
    }
    t_team* team = join_team();
    if(team) join_board(team);
    if(data.first && !data.joined) data.abort = YES;

    if(semtimedop(data.semid, &data.sem->players_count_unlock, 1, &data.sem->timeout)
            || semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)
            || semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout))
        return failure("semtimedop");

    return data.joined ? EXIT_SUCCESS : EXIT_FAILURE;
}
