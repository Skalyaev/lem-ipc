#include "../include/header.h"

extern t_lemipc data;

static void clean_ipc(t_team teams[MAX_TEAMS]) {

    if(data.shmid > 0 && shmctl(data.shmid, IPC_RMID, NULL)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("clean_ipc: shmctl");
    }
    if(data.semid > 0 && semctl(data.semid, 0, IPC_RMID)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("clean_ipc: semctl");
    }
    if(access(SHM_PATH, F_OK) == 0 && remove(SHM_PATH)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("clean_ipc: remove");
    }
    if(access(SEM_PATH, F_OK) == 0 && remove(SEM_PATH)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("clean_ipc: remove");
    }
    char path[64] = {0};
    for(ubyte x = 0; x < MAX_TEAMS; x++) {

        if(teams[x].msgid == -1) continue;
        if(msgctl(teams[x].msgid, IPC_RMID, NULL)) {

            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("clean_ipc: msgctl");
        }
        sprintf(path, MSG_PATH, x);
        if(access(path, F_OK)) continue;

        if(!remove(path)) continue;
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("clean_ipc: remove");
    }
}

static void leave_ipc(ubyte* players_left) {

    bool canstop = NO;
    if(!semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        data.shm->players_count--;
        *players_left = data.shm->players_count;
        if(data.shm->players_count < 3) canstop = YES;

        t_team* myteam = NULL;
        t_team* team;
        bool low_teams = YES;
        for(ubyte x = 0; x < MAX_TEAMS; x++) {

            team = &data.shm->teams[x];
            if(!team->name[0]) continue;
            if(data.self.color == team->color) {

                team->players_count--;
                myteam = team;
            }
            if(team->players_count > 1) low_teams = NO;
            if(team->players_count) continue;

            data.shm->teams_count--;
            memset(team->name, 0, NAME_SIZE);
        }
        if(low_teams) canstop = YES;
        if(data.shm->teams_count < 2) canstop = YES;

        for(ubyte x = 0; x < myteam->players_count + 1; x++) {

            if(myteam->ids[x] != data.self.id) continue;
            myteam->ids[x] = 0;

            for(ubyte y = x; y < myteam->players_count + 1; y++)
                myteam->ids[y] = myteam->ids[y + 1];
            break;
        }
        if(semtimedop(data.semid, &data.sem->teams_unlock,
                      1, &data.sem->timeout)) {

            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("leave_ipc: semtimedop");
        }
    } else {
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("leave_ipc: semtimedop");
    }
    t_player* player;
    if(!semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        for(ubyte x = 0; x < BOARD_WIDTH; x++) {
            for(ubyte y = 0; y < BOARD_HEIGHT; y++) {

                player = &data.shm->board[x][y];
                if(player->id != data.self.id) continue;

                player->id = 0;
                player->color = 0;
            }
        }
        if(semtimedop(data.semid, &data.sem->board_unlock,
                      1, &data.sem->timeout)) {

            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("leave_ipc: semtimedop");
        }
    } else {
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("leave_ipc: semtimedop");
    }
    if(!canstop) return;

    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("leave_ipc: semtimedop");
        return;
    }
    if(data.shm->started && !data.shm->over) {

        gettimeofday(&data.shm->end, NULL);
        data.shm->over = YES;
    }
    if(semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("leave_ipc: semtimedop");
        return;
    }
}

static byte clean_wm() {

    t_team teams[MAX_TEAMS] = {0};
    const size_t team_size = sizeof(teams);

    ubyte players_left = -1;
    if(data.is_sub) leave_ipc(&players_left);
    else {
        pid_t sub;
        for(ubyte x = 0; x < data.wm->subs_count; x++) {
            sub = data.wm->subs[x];

            if (kill(sub, 0) == -1) continue;
            if (kill(sub, SIGTERM) == -1) continue;

            int status;
            if (waitpid(sub, &status, 0) == -1) continue;
        }
        if(!semtimedop(data.semid, &data.sem->gui_lock,
                       1, &data.sem->timeout)) {

            data.shm->gui = NO;
            if(semtimedop(data.semid, &data.sem->gui_unlock,
                          1, &data.sem->timeout)) {

                if(data.code == EXIT_SUCCESS) data.code = errno;
                perror("clean_wm: semtimedop");
            }
        } else {
            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("clean_wm: semtimedop");
        }
        if(!semtimedop(data.semid, &data.sem->teams_lock,
                       1, &data.sem->timeout)) {

            players_left = data.shm->players_count;
            memcpy(teams, data.shm->teams, team_size);

            if(semtimedop(data.semid, &data.sem->teams_unlock,
                          1, &data.sem->timeout)) {

                if(data.code == EXIT_SUCCESS) data.code = errno;
                perror("clean_wm: semtimedop");
            }
        } else {
            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("clean_wm: semtimedop");
        }
    }
    if(data.shm && data.shm != (void*)-1 && shmdt(data.shm)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("clean_wm: shmdt");
    }
    if(!data.is_sub) {

        if(data.abort) clean_ipc(teams);
        else if(!players_left) clean_ipc(teams);
    }
    if(data.sem) free(data.sem);

    if(data.wm->screen.img) mlx_destroy_image(data.wm->mlx, data.wm->screen.img);
    if(data.wm->win) mlx_destroy_window(data.wm->mlx, data.wm->win);
    if(data.wm->mlx) {

        mlx_loop_end(data.wm->mlx);
        mlx_set_font(data.wm->mlx, NULL, NULL);
        mlx_destroy_display(data.wm->mlx);
        free(data.wm->mlx);
    }
    free(data.wm);
    return data.code;
}

byte bye() {

    if(data.wm) return clean_wm();
    bool gui = NO;
    ubyte players_left = -1;

    t_team teams[MAX_TEAMS] = {0};
    const size_t team_size = sizeof(teams);

    if(!semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        memcpy(teams, data.shm->teams, team_size);
        if(semtimedop(data.semid, &data.sem->teams_unlock,
                      1, &data.sem->timeout)) {

            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("bye: semtimedop");
        }
    } else {
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("bye: semtimedop");
    }
    if(!semtimedop(data.semid, &data.sem->gui_lock, 1, &data.sem->timeout)) {

        gui = data.shm->gui;
        if(data.joined) leave_ipc(&players_left);

        if(semtimedop(data.semid, &data.sem->gui_unlock,
                      1, &data.sem->timeout)) {

            if(data.code == EXIT_SUCCESS) data.code = errno;
            perror("bye: semtimedop");
        }
    } else {
        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("bye: semtimedop");
    }
    if(data.shm && data.shm != (void*)-1 && shmdt(data.shm)) {

        if(data.code == EXIT_SUCCESS) data.code = errno;
        perror("bye: shmdt");
    }
    if(data.abort) clean_ipc(teams);
    else if(!players_left && !gui) clean_ipc(teams);

    if(data.sem) free(data.sem);
    return data.code;
}
