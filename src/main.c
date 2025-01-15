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

    const size_t wm_size = sizeof(t_mlx);
    data.wm = malloc(wm_size);
    if(!data.wm) {

        perror("main: malloc");
        data.code = errno;
        return bye();
    }
    memset(data.wm, 0, wm_size);
    data.wm->mlx = mlx_init();

    int sizex = 0;
    int sizey = 0;
    mlx_get_screen_size(data.wm->mlx, &sizex, &sizey);
    if(data.shm->width * PIXEL_SIZE > sizex
            || data.shm->height * PIXEL_SIZE > sizey) {

        mlx_destroy_display(data.wm->mlx);
        free(data.wm->mlx);
        free(data.wm);
        data.wm = NULL;
        if(data.first) data.abort = YES;

        fprintf(stderr, RED"Board too large: %dx%d < %dx%d\n"RESET,
                sizex, sizey, data.shm->width * PIXEL_SIZE,
                data.shm->height * PIXEL_SIZE);
        return bye();
    }
    if(data.opt.gui) return draw();

    mlx_destroy_display(data.wm->mlx);
    free(data.wm->mlx);
    free(data.wm);
    data.wm = NULL;

    if(join() != EXIT_SUCCESS) return bye();
    ubyte idx = 0;
    bool loop;
    while(YES) {

        loop = YES;
        if(semtimedop(data.semid, &data.sem->party_lock,
                      1, &data.sem->timeout)) {

            perror("main: semtimedop");
            data.code = errno;
            return bye();
        }
        if(!data.shm->started || data.shm->paused || data.shm->over) {

            printf("Waiting for game to start...\n");
            loop = NO;
            usleep(100000);
        }
        if(semtimedop(data.semid, &data.sem->party_unlock,
                      1, &data.sem->timeout)) {

            perror("main: semtimedop");
            data.code = errno;
            return bye();
        }
        if(!loop) continue;

        printf("Player %d is checking...\n", data.self->id);
        if(player_check() != EXIT_SUCCESS) return bye();
        printf("Player %d is thinking...\n", data.self->id);
        if(player_think() != EXIT_SUCCESS) return bye();
        printf("Player %d is communicating...\n", data.self->id);
        if(player_communicate() != EXIT_SUCCESS) return bye();
        printf("Player %d is moving...\n", data.self->id);
        if(player_move() != EXIT_SUCCESS) return bye();
        if(!data.opt.quiet) {

            if(idx % 16 == 0)
                if(player_log() != EXIT_SUCCESS) return bye();
            idx++;
        }
        usleep(100000);
    }
    return bye();
}
