#include "../include/header.h"

extern t_lemipc data;

static byte failure(const char* const msg) {

    if(data.first) data.abort = YES;
    data.code = errno;
    perror(msg);
    return EXIT_FAILURE;
}

static byte try_cell(const pid_t* const pid,
                     const ushort* const x,
                     const ushort* const y,
                     t_team* const team) {

    t_player* const player = &data.shm->board[*x][*y];
    if(player->id) return EXIT_FAILURE;

    player->id = *pid;
    player->color = team->color;

    team->ids[team->players_count] = *pid;
    team->players_count++;
    data.shm->players_count++;

    memcpy(&data.self, player, PLAYER_SIZE);
    data.joined = YES;
    return EXIT_SUCCESS;
}

static void join_board(t_team* const team) {

    const pid_t pid = getpid();

    srand(time(NULL) + pid);
    const ushort baseX = rand() % BOARD_WIDTH;
    const ushort baseY = rand() % BOARD_HEIGHT;

    ushort x = baseX + 1;
    ushort y = baseY + 1;
    while(YES) {

        if(x >= BOARD_WIDTH) {
            x = 0;
            y++;
        }
        if(y >= BOARD_HEIGHT) y = 0;

        if(try_cell(&pid, &x, &y, team) == EXIT_SUCCESS) return;
        x++;
        if(x == baseX && y == baseY) break;
    }
    if(team->players_count) return;
    data.shm->teams_count--;
    memset(team->name, 0, NAME_SIZE);
}

static t_team* join_team() {

    if(data.shm->players_count == MAX_PLAYERS) return NULL;
    t_team* team = NULL;
    for(ubyte x = 0; x < MAX_TEAMS; x++) {

        if(strcmp(data.opt.team, data.shm->teams[x].name)) continue;
        team = &data.shm->teams[x];
        break;
    }
    if(team) return team;
    for(ubyte x = 0; x < MAX_TEAMS; x++) {

        if(data.shm->teams[x].players_count) continue;
        data.shm->teams_count++;

        team = &data.shm->teams[x];
        strcpy(team->name, data.opt.team);
        break;
    }
    return team;
}

byte join() {

    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout))
        return failure("join: semtimedop");

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout);
        return failure("join: semtimedop");
    }
    t_team* const team = join_team();
    if(team) join_board(team);

    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)
            || semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout))
        return failure("join: semtimedop");

    if(!data.joined) return EXIT_FAILURE;

    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout))
        return failure("join: semtimedop");

    if((!data.shm->started || data.shm->over)
            && data.shm->players_count > 2 && data.shm->teams_count > 1) {

        t_team* team;
        for(ubyte x = 0; x < MAX_TEAMS; x++) {

            team = &data.shm->teams[x];
            if(team->players_count < 2) continue;

            gettimeofday(&data.shm->start, NULL);
            data.shm->over = NO;
            data.shm->started = YES;
            break;
        }
    }
    if(semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout))
        return failure("join: semtimedop");
    return EXIT_SUCCESS;
}
