#include "../include/header.h"

extern t_lemipc data;

void getargs(const int ac, char** const av) {

    data.opt.width = MAX_WIDTH;
    data.opt.height = MAX_HEIGHT;
    data.opt.max_teams = MAX_TEAMS;
    data.opt.max_players = MAX_PLAYERS;

    const char* const optstring = "gqh";
    const t_option options[] = {

        {"gui", no_argument, 0, 'g'},
        {"quiet", no_argument, 0, 'q'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int idx = 0;
    int opt;
    while((opt = getopt_long(ac, av, optstring, options, &idx)) != -1) {

        switch(opt) {
        case 'g':
            data.opt.gui = YES;
            break;
        case 'q':
            data.opt.quiet = YES;
            break;
        case 'h':
            printf(USAGE, av[0]);
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, RED"Invalid option: %s\n"RESET, av[optind]);
            printf(USAGE, av[0]);
            exit(EXIT_FAILURE);
        }
    }
    if(data.opt.gui) return;
    for(int x = optind; x < ac; x++) {

        if(data.opt.team) {

            fprintf(stderr, RED"Invalid argument: %s\n"RESET, av[x]);
            printf(USAGE, av[0]);
            exit(EXIT_FAILURE);
        }
        if(!av[x][0] || strlen(av[x]) > NAME_SIZE_) {

            fprintf(stderr, RED"Invalid team name: %s\n"RESET, av[x]);
            printf(USAGE, av[0]);
            exit(EXIT_FAILURE);
        }
        data.opt.team = av[x];
    }
    if(data.opt.team) return;

    fprintf(stderr, RED"Missing argument: TEAM\n"RESET);
    printf(USAGE, av[0]);
    exit(EXIT_FAILURE);
}
