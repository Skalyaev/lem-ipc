#include "../include/header.h"

extern t_lemipc data;

byte add_player(const ubyte teamidx) {

    if(data.subs_count == MAX_SUBS) return EXIT_SUCCESS;

    char name[NAME_SIZE] = {0};
    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

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
    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

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

    char* const av[] = {"./lemipc", name, NULL};
    extern char** environ;
    execve(av[0], av, environ);
    perror("add_player: execve");
    exit(EXIT_FAILURE);
}
