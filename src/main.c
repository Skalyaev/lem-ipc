#include "../include/header.h"

t_lemipc data = {0};

static void sigexit(int sig) {

    static bool exiting = NO;
    if(exiting) return;
    exiting = YES;

    if(data.first && !data.joined) data.abort = YES;
    data.code = sig;
    exit(bye());
}

int main(int ac, char** av) {

    signal(SIGINT, sigexit);
    signal(SIGTERM, sigexit);
    signal(SIGQUIT, sigexit);
    getargs(ac, av);
    if(init() != EXIT_SUCCESS) return bye();

    if(data.opt.gui) return draw();
    else if(join() != EXIT_SUCCESS) return bye();
    while(YES) {

        printf("\nPosition: %d, %d\n", data.self->x, data.self->y);
        printf("Team: %s\n", data.self->team->name);

        if(semtimedop(data.semid, &data.sem->players_count_lock, 1, &data.sem->timeout)) {
            perror("semtimedop");
            return bye();
        }
        printf("Teammates: %d\n", data.self->team->players_count);
        if(semtimedop(data.semid, &data.sem->players_count_unlock, 1, &data.sem->timeout)) {
            perror("semtimedop");
            return bye();
        }
        if(semtimedop(data.semid, &data.sem->players_count_lock, 1, &data.sem->timeout)) {
            perror("semtimedop");
            return bye();
        }
        printf("Players count: %d\n", data.shm->players_count);
        if(semtimedop(data.semid, &data.sem->players_count_unlock, 1, &data.sem->timeout)) {
            perror("semtimedop");
            return bye();
        }
        if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {
            perror("semtimedop");
            return bye();
        }
        printf("Teams count: %d\n", data.shm->teams_count);
        if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {
            perror("semtimedop");
            return bye();
        }
        sleep(2);
    }
    return bye();
}
