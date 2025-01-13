#include "../include/header.h"

extern t_lemipc data;

byte player_check() {

    return EXIT_SUCCESS;
}

byte player_listen() {

    return EXIT_SUCCESS;
}

byte player_think() {

    return EXIT_SUCCESS;
}

byte player_communicate() {

    return EXIT_SUCCESS;
}

byte player_move() {

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

    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_team* team = NULL;
    for(ubyte x = 0; x < data.shm->max_teams; x++) {

        if(!data.shm->teams[x].name[0]) continue;
        if(!strcmp(data.shm->teams[x].name, data.opt.team)) {

            team = &data.shm->teams[x];
            break;
        }
    }
    printf("\nTeam: %s%s%s\n", team_color(), team->name, RESET);
    printf("Team mates: %d\n", team->players_count);
    printf("Teams count: %d\n", data.shm->teams_count);

    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    if(semtimedop(data.semid, &data.sem->players_count_lock, 1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    printf("Players count: %d\n", data.shm->players_count);

    if(semtimedop(data.semid, &data.sem->players_count_unlock, 1, &data.sem->timeout)) {

        perror("player_log: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    printf("Position: %d, %d\n", data.self->x, data.self->y);
    return EXIT_SUCCESS;
}
