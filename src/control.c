#include "../include/header.h"

extern t_lemipc data;

byte add_player(const ubyte teamidx) {

    if(data.subs_count == MAX_SUBS) return EXIT_SUCCESS;

    char name[NAME_SIZE] = {0};
    if(semtimedop(data.semid, &data.sem->teams_lock,
                  1, &data.sem->timeout)) {

        perror("add_player: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_team* team = &data.shm->teams[teamidx];
    if(!team->name[0]) {

        sprintf(team->name, "team%d", teamidx + 1);
        data.shm->teams_count++;
    }
    strcpy(name, team->name);
    if(semtimedop(data.semid, &data.sem->teams_unlock,
                  1, &data.sem->timeout)) {

        perror("add_player: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    data.subs[data.subs_count] = fork();
    if(data.subs[data.subs_count] == -1) {

        perror("add_player: fork");
        data.code = errno;
        return EXIT_FAILURE;
    }
    data.subs_count++;
    if(data.subs[data.subs_count - 1]) return EXIT_SUCCESS;
    data.is_sub = YES;

    char* const av[] = {"./lemipc", name, NULL};
    extern char** environ;

    execve(av[0], av, environ);
    perror("add_player: execve");
    data.code = errno;
    exit(bye());
}

byte pause_game() {

    static const size_t timeval_size = sizeof(t_timeval);

    if(semtimedop(data.semid, &data.sem->party_lock,
                  1, &data.sem->timeout)) {

        perror("pause_game: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    data.shm->paused = !data.shm->paused;
    if(data.shm->paused) gettimeofday(&data.shm->pause, NULL);
    else {
        if(data.shm->started) {
            if(data.shm->start.tv_sec < data.shm->pause.tv_sec) {

                t_timeval now, diff;
                gettimeofday(&now, NULL);
                timersub(&now, &data.shm->pause, &diff);
                timeradd(&data.shm->start, &diff, &data.shm->start);
            }
            else gettimeofday(&data.shm->start, NULL);
        }
        memset(&data.shm->pause, 0, timeval_size);
    }
    if(semtimedop(data.semid, &data.sem->party_unlock,
                  1, &data.sem->timeout)) {

        perror("pause_game: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

byte stop_game() {

    if(semtimedop(data.semid, &data.sem->party_lock,
                  1, &data.sem->timeout)) {

        perror("stop_game: semtimedop");
        data.code = errno;
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
            memset(&data.shm->pause, 0, sizeof(t_timeval));
        }
        gettimeofday(&data.shm->end, NULL);
    }
    if(semtimedop(data.semid, &data.sem->party_unlock,
                  1, &data.sem->timeout)) {

        perror("stop_game: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
