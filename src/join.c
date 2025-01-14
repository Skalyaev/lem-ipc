#include "../include/header.h"

extern t_lemipc data;

static byte failure(const char* const msg) {

    perror(msg);
    data.code = errno;
    if(data.first) data.abort = YES;
    return EXIT_FAILURE;
}

static bool try_cell(ushort* const x, ushort* const y, t_team* const team) {

    if(data.shm->board[*x][*y].id) return NO;

    data.self = &data.shm->board[*x][*y];
    data.self->x = *x;
    data.self->y = *y;
    data.self->color = team->color;
    data.self->id = data.shm->players_count + 1;

    team->players_count++;
    data.shm->players_count++;
    data.joined = YES;
    return YES;
}

static void join_board(t_team* const team) {

    const ushort board_width = data.shm->width / PLAYER_VOLUME;
    const ushort board_height = data.shm->height / PLAYER_VOLUME;

    srand(time(NULL) + getpid());
    ushort x = rand() % board_width;
    ushort y = rand() % board_height;

    if(x > board_width) x = 0;
    if(y > board_width) y = 0;

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) return;
    while(YES) {

        if(try_cell(&x, &y, team)) break;
        x++;
        if(x <= board_width) continue;
        x = 0;
        y++;
        if(y > board_height) y = 0;
    }
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) return;
}

static t_team* join_team() {

    if(data.shm->players_count == data.shm->max_players) return NULL;
    t_team* team = NULL;
    for(ushort x = 0; x < data.shm->max_teams; x++) {

        if(strcmp(data.opt.team, data.shm->teams[x].name)) continue;
        team = &data.shm->teams[x];
        break;
    }
    if(team) return team;
    for(ushort x = 0; x < data.shm->max_teams; x++) {

        if(data.shm->teams[x].name[0]) continue;
        strcpy(data.shm->teams[x].name, data.opt.team);
        team = &data.shm->teams[x];
        data.shm->teams_count++;
        break;
    }
    return team;
}

byte join() {

    if(semtimedop(data.semid, &data.sem->init_lock, 1, &data.sem->timeout))
        return failure("join: semtimedop");

    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout);
        return failure("join: semtimedop");
    }
    if(semtimedop(data.semid, &data.sem->players_count_lock, 1, &data.sem->timeout)) {

        semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout);
        semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout);
        return failure("join: semtimedop");
    }
    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout)) {

        semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout);
        semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout);
        semtimedop(data.semid, &data.sem->players_count_unlock, 1, &data.sem->timeout);
        return failure("join: semtimedop");
    }
    t_team* const team = join_team();
    if(team) join_board(team);
    if(data.joined) {

        bool canstart = NO;
        for(ubyte x = 0; x < data.shm->max_teams; x++) {

            if(!data.shm->teams[x].name[0]) continue;
            if(data.shm->teams[x].players_count < 2) continue;
            canstart = YES;
            break;
        }
        if(canstart && (data.shm->over || !data.shm->started)
                && data.shm->players_count > 2
                && data.shm->teams_count > 1) {

            data.shm->started = YES;
            data.shm->over = NO;
            gettimeofday(&data.shm->start, NULL);
        }
    } else if(data.first) data.abort = YES;

    if(semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout)
            || semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)
            || semtimedop(data.semid, &data.sem->players_count_unlock, 1, &data.sem->timeout)
            || semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout))
        return failure("join: semtimedop");

    return data.joined ? EXIT_SUCCESS : EXIT_FAILURE;
}
