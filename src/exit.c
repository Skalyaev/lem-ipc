#include "../include/header.h"

extern t_lemipc data;

static void leave_ipc(ubyte* const players_left) {

    if(!semtimedop(data.semid, &data.sem->players_count_lock,
                   1, &data.sem->timeout)) {

        data.shm->players_count--;
        *players_left = data.shm->players_count;

        if(semtimedop(data.semid, &data.sem->players_count_unlock,
                      1, &data.sem->timeout)) {

            perror("semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        if(data.first) data.abort = YES;
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("semtimedop");
    }
    if(!semtimedop(data.semid, &data.sem->teams_lock,
                   1, &data.sem->timeout)) {

        data.self->team->players_count--;
        if(!data.self->team->players_count) {

            memset(data.self->team->name, 0, NAME_SIZE);
            data.shm->teams_count--;
        }
        if(semtimedop(data.semid, &data.sem->teams_unlock,
                      1, &data.sem->timeout)) {

            perror("semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        if(data.first) data.abort = YES;
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("semtimedop");
    }
    if(!semtimedop(data.semid, &data.sem->board_lock,
                   1, &data.sem->timeout)) {

        for(ushort x = data.self->x; x < data.self->x + PLAYER_VOLUME; x++)
            for(ushort y = data.self->y; y < data.self->y + PLAYER_VOLUME; y++)
                data.shm->board[x][y].team = NULL;

        if(semtimedop(data.semid, &data.sem->board_unlock,
                      1, &data.sem->timeout)) {

            perror("semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        if(data.first) data.abort = YES;
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("semtimedop");
    }
}

static void clean_ipc() {

    if(shmctl(data.shmid, IPC_RMID, NULL)) {

        perror("shmctl");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(data.semid > 0 && semctl(data.semid, 0, IPC_RMID)) {

        perror("semctl");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(access(SHM_PATH, F_OK) == 0 && remove(SHM_PATH)) {

        perror("remove");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(access(SEM_PATH, F_OK) == 0 && remove(SEM_PATH)) {

        perror("remove");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
}

static byte clean_wm() {

    ubyte players_left = -1;
    if(!semtimedop(data.semid, &data.sem->gui_lock,
                   1, &data.sem->timeout)) {

        data.shm->gui = NO;
        if(semtimedop(data.semid, &data.sem->gui_unlock,
                      1, &data.sem->timeout)) {

            perror("semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        perror("semtimedop");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(!semtimedop(data.semid, &data.sem->players_count_lock,
                   1, &data.sem->timeout)) {

        players_left = data.shm->players_count;
        if(semtimedop(data.semid, &data.sem->players_count_unlock,
                      1, &data.sem->timeout)) {

            perror("semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        perror("semtimedop");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(data.wm->screen.img) mlx_destroy_image(data.wm->mlx, data.wm->screen.img);
    if(data.wm->win) mlx_destroy_window(data.wm->mlx, data.wm->win);
    if(data.wm->mlx) {

        mlx_destroy_display(data.wm->mlx);
        free(data.wm->mlx);
    }
    free(data.wm);
    if(shmdt(data.shm)) {

        perror("shmdt");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(!players_left) clean_ipc();
    if(data.sem) free(data.sem);
    return data.code;
}

byte bye() {

    if(!data.shm || data.shm == (void*)-1) return data.code;
    if(data.wm) return clean_wm();

    ubyte players_left = -1;
    bool gui = NO;
    if(!semtimedop(data.semid, &data.sem->gui_lock,
                   1, &data.sem->timeout)) {

        gui = data.shm->gui;
        if(semtimedop(data.semid, &data.sem->gui_unlock, 1,
                      &data.sem->timeout)) {

            perror("semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        perror("semtimedop");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(data.joined) leave_ipc(&players_left);
    if(shmdt(data.shm)) {

        perror("shmdt");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(data.abort) clean_ipc();
    else if(!players_left && !gui) clean_ipc();

    if(data.sem) free(data.sem);
    return data.code;
}
