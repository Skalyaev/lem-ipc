#include "../include/header.h"

extern t_lemipc data;

static void clean_ipc() {

    if(shmctl(data.shmid, IPC_RMID, NULL)) {

        perror("clean_ipc: shmctl");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(data.semid > 0 && semctl(data.semid, 0, IPC_RMID)) {

        perror("clean_ipc: semctl");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(access(SHM_PATH, F_OK) == 0 && remove(SHM_PATH)) {

        perror("clean_ipc: remove");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(access(SEM_PATH, F_OK) == 0 && remove(SEM_PATH)) {

        perror("clean_ipc: remove");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(access(MSG_PATH, F_OK) == 0 && remove(MSG_PATH)) {

        perror("clean_ipc: remove");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
}

static void leave_ipc(ubyte* const players_left) {

    bool canstop = NO;
    if(!semtimedop(data.semid, &data.sem->players_count_lock,
                   1, &data.sem->timeout)) {

        data.shm->players_count--;
        *players_left = data.shm->players_count;
        if(data.shm->players_count < 3) canstop = YES;

        if(semtimedop(data.semid, &data.sem->players_count_unlock,
                      1, &data.sem->timeout)) {

            perror("leave_ipc: semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        perror("leave_ipc: semtimedop");
        if(data.first) data.abort = YES;
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(!semtimedop(data.semid, &data.sem->teams_lock,
                   1, &data.sem->timeout)) {

        t_team* team = NULL;
        for(ubyte x = 0; x < data.shm->max_teams; x++) {

            if(!data.shm->teams[x].name[0]) continue;
            if(!strcmp(data.shm->teams[x].name, data.opt.team)) {

                team = &data.shm->teams[x];
                break;
            }
            if(data.shm->teams[x].players_count) continue;
            data.shm->teams_count--;

            char path[64] = {0};
            sprintf(path, MSG_PATH, x);
            if(data.shm->teams[x].msgid){

                if(msgctl(data.shm->teams[x].msgid, IPC_RMID, NULL)) {

                    perror("leave_ipc: msgctl");
                    if(data.code == EXIT_SUCCESS) data.code = errno;
                }
                if(access(path, F_OK) == 0 && remove(path)) {

                    perror("leave_ipc: remove");
                    if(data.code == EXIT_SUCCESS) data.code = errno;
                }
            }
            memset(data.shm->teams[x].name, 0, NAME_SIZE);
        }
        for(ubyte x = 0; x < team->players_count; x++) {

            if(team->ids[x] != data.self->id) continue;
            ubyte y = x;

            for(; y < team->players_count - 1; y++)
                team->ids[y] = team->ids[y + 1];

            team->ids[y] = 0;
            break;
        }
        team->players_count--;
        if(!team->players_count) {

            if(team->msgid > 0 && msgctl(team->msgid, IPC_RMID, NULL)) {

                perror("clean_ipc: msgctl");
                if(data.code == EXIT_SUCCESS) data.code = errno;
            }
            memset(team->name, 0, NAME_SIZE);
            data.shm->teams_count--;
            if(data.shm->teams_count < 2) canstop = YES;
        }
        if(semtimedop(data.semid, &data.sem->teams_unlock,
                      1, &data.sem->timeout)) {

            perror("leave_ipc: semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        perror("leave_ipc: semtimedop");
        if(data.first) data.abort = YES;
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(!semtimedop(data.semid, &data.sem->board_lock,
                   1, &data.sem->timeout)) {

        data.self->id = 0;
        data.self->color = 0;
        if(semtimedop(data.semid, &data.sem->board_unlock,
                      1, &data.sem->timeout)) {

            perror("leave_ipc: semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else {
        perror("leave_ipc: semtimedop");
        if(data.first) data.abort = YES;
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(!canstop) return;

    if(semtimedop(data.semid, &data.sem->party_lock,
                  1, &data.sem->timeout)) {

        perror("leave_ipc: semtimedop");
        if(data.code == EXIT_SUCCESS) data.code = errno;
        return;
    }
    if(data.shm->started && !data.shm->over) {

        gettimeofday(&data.shm->end, NULL);
        data.shm->over = YES;
    }
    if(semtimedop(data.semid, &data.sem->party_unlock,
                  1, &data.sem->timeout)) {

        perror("leave_ipc: semtimedop");
        if(data.code == EXIT_SUCCESS) data.code = errno;
        return;
    }
}

static byte clean_wm() {

    ubyte players_left = -1;
    if(!data.is_sub) {

        for(ubyte x = 0; x < data.subs_count; x++) {

            kill(data.subs[x], SIGTERM);
            waitpid(data.subs[x], NULL, 0);
        }
        if(!semtimedop(data.semid, &data.sem->gui_lock,
                       1, &data.sem->timeout)) {

            data.shm->gui = NO;
            if(semtimedop(data.semid, &data.sem->gui_unlock,
                          1, &data.sem->timeout)) {

                perror("clean_wm: semtimedop");
                if(data.code == EXIT_SUCCESS) data.code = errno;
            }
        } else {
            perror("clean_wm: semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
        if(!semtimedop(data.semid, &data.sem->players_count_lock,
                       1, &data.sem->timeout)) {

            players_left = data.shm->players_count;
            if(semtimedop(data.semid, &data.sem->players_count_unlock,
                          1, &data.sem->timeout)) {

                perror("clean_wm: semtimedop");
                if(data.code == EXIT_SUCCESS) data.code = errno;
            }
        } else if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    else leave_ipc(&players_left);

    if(data.wm->screen.img) mlx_destroy_image(data.wm->mlx, data.wm->screen.img);
    if(data.wm->win) mlx_destroy_window(data.wm->mlx, data.wm->win);
    if(data.wm->mlx) {

        mlx_loop_end(data.wm->mlx);
        mlx_set_font(data.wm->mlx, NULL, NULL);
        mlx_destroy_display(data.wm->mlx);
        free(data.wm->mlx);
    }
    free(data.wm);
    if(shmdt(data.shm)) {

        perror("clean_wm: shmdt");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(!data.is_sub && !players_left) clean_ipc();
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

            perror("bye: semtimedop");
            if(data.code == EXIT_SUCCESS) data.code = errno;
        }
    } else if(data.code == EXIT_SUCCESS) {

        perror("bye: semtimedop");
        data.code = errno;
    }
    if(data.joined) leave_ipc(&players_left);
    if(shmdt(data.shm)) {

        perror("bye: shmdt");
        if(data.code == EXIT_SUCCESS) data.code = errno;
    }
    if(data.abort) clean_ipc();
    else if(!players_left && !gui) clean_ipc();

    if(data.sem) free(data.sem);
    return data.code;
}
