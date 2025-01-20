#include "../include/header.h"

extern t_lemipc data;

byte pause_game() {

    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("pause_game: semtimedop");
        return EXIT_FAILURE;
    }
    data.shm->paused = !data.shm->paused;
    if(data.shm->paused) gettimeofday(&data.shm->pause, NULL);
    else {
        if(data.shm->started && !data.shm->over) {
            if(data.shm->start.tv_sec < data.shm->pause.tv_sec) {

                t_timeval now, diff;
                gettimeofday(&now, NULL);
                timersub(&now, &data.shm->pause, &diff);
                timeradd(&data.shm->start, &diff, &data.shm->start);
            }
            else gettimeofday(&data.shm->start, NULL);
        }
        memset(&data.shm->pause, 0, TIMEVAL_SIZE);
    }
    if(semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("pause_game: semtimedop");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

byte stop_game() {

    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("stop_game: semtimedop");
        return EXIT_FAILURE;
    }
    if(data.shm->started && !data.shm->over) {

        data.shm->over = YES;
        if(data.shm->paused) {

            t_timeval now, diff;
            gettimeofday(&now, NULL);
            timersub(&now, &data.shm->pause, &diff);

            timeradd(&data.shm->start, &diff, &data.shm->start);
            data.shm->paused = NO;
            memset(&data.shm->pause, 0, TIMEVAL_SIZE);
        }
        gettimeofday(&data.shm->end, NULL);
    }
    if(semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("stop_game: semtimedop");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

byte add_player(const ubyte teamidx) {

    if(data.wm->subs_count == MAX_SUBS) return EXIT_SUCCESS;

    char name[NAME_SIZE] = {0};
    bool ok = NO;
    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("add_player: semtimedop");
        return EXIT_FAILURE;
    }
    if(data.shm->players_count < MAX_PLAYERS) {

        t_team* team = &data.shm->teams[teamidx];
        if(!team->name[0]) {

            sprintf(team->name, "team%d", teamidx + 1);
            data.shm->teams_count++;
        }
        strcpy(name, team->name);
        ok = YES;
    }
    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("add_player: semtimedop");
        return EXIT_FAILURE;
    }
    if(!ok) return EXIT_SUCCESS;
    const ushort subs_count = data.wm->subs_count;

    data.wm->subs[subs_count] = fork();
    if(data.wm->subs[subs_count] == -1) {

        data.code = errno;
        perror("add_player: fork");
        return EXIT_FAILURE;
    }
    data.wm->subs_count++;
    if(data.wm->subs[subs_count]) return EXIT_SUCCESS;
    data.is_sub = YES;

    char* const av[] = {"./lemipc", "-q", name, NULL};
    extern char** environ;

    execve(av[0], av, environ);
    data.code = errno;
    perror("add_player: execve");
    exit(bye());
}
