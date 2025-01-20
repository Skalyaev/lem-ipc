#include "../include/header.h"

t_lemipc data = {0};

static void sigexit(int sig) {

    static bool exiting = NO;
    if(exiting) return;
    exiting = YES;

    if(data.first && !data.joined)
        data.abort = YES;

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
    if(join() != EXIT_SUCCESS) return bye();

    ubyte idx = 0;
    bool loop;
    while(YES) {

        loop = YES;
        if(semtimedop(data.semid, &data.sem->party_lock,
                      1, &data.sem->timeout)) {

            data.code = errno;
            perror("main: semtimedop");
            return bye();
        }
        if(!data.shm->started || data.shm->paused || data.shm->over) {

            loop = NO;
            if(!data.opt.quiet) printf("Waiting for game to start...\n");
            usleep(100000);
        }
        if(semtimedop(data.semid, &data.sem->party_unlock,
                      1, &data.sem->timeout)) {

            data.code = errno;
            perror("main: semtimedop");
            return bye();
        }
        if(!loop) continue;

        if(player_check() != EXIT_SUCCESS) break;
        if(player_think() != EXIT_SUCCESS) break;
        if(player_communicate() != EXIT_SUCCESS) break;
        if(player_move() != EXIT_SUCCESS) break;

        if(!data.opt.quiet && idx++ % 16 == 0
                && player_log() != EXIT_SUCCESS) break;

        usleep(100000);
    }
    return bye();
}
