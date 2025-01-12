#include "../include/header.h"

extern t_lemipc data;

void getargs(const int ac, char** const av) {

    data.opt.width = MAX_WIDTH;
    data.opt.height = MAX_HEIGHT;
    data.opt.max_teams = MAX_TEAMS;
    data.opt.max_players = MAX_PLAYERS;

    const char* const optstring = "W:H:t:p:gh";
    const t_option options[] = {

        {"width", required_argument, 0, 'W'},
        {"height", required_argument, 0, 'H'},
        {"max-teams", required_argument, 0, 't'},
        {"max-players", required_argument, 0, 'p'},
        {"gui", no_argument, 0, 'g'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };
    int idx = 0;
    int opt;
    while((opt = getopt_long(ac, av, optstring, options, &idx)) != -1) {

        switch(opt) {
        case 'W':
            data.opt.width = atoi(optarg);
            if(!data.opt.width || data.opt.width > MAX_WIDTH) {

                fprintf(stderr, RED"Invalid width: %d\n"RESET, data.opt.width);
                printf(USAGE, av[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'H':
            data.opt.height = atoi(optarg);
            if(!data.opt.height || data.opt.height > MAX_HEIGHT) {

                fprintf(stderr, RED"Invalid height: %d\n"RESET, data.opt.height);
                printf(USAGE, av[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 't':
            data.opt.max_teams = atoi(optarg);
            if(data.opt.max_teams < 2 || data.opt.max_teams > MAX_TEAMS) {

                fprintf(stderr, RED"Invalid max teams: %d\n"RESET, data.opt.max_teams);
                printf(USAGE, av[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'p':
            data.opt.max_players = atoi(optarg);
            if(data.opt.max_players < 2 || data.opt.max_players > MAX_PLAYERS) {

                fprintf(stderr, RED"Invalid max players: %d\n"RESET, data.opt.max_players);
                printf(USAGE, av[0]);
                exit(EXIT_FAILURE);
            }
            break;
        case 'g':
            data.opt.gui = YES;
            return;
        case 'h':
            printf(USAGE, av[0]);
            exit(EXIT_SUCCESS);
        default:
            fprintf(stderr, RED"Invalid option: %s\n"RESET, av[optind]);
            printf(USAGE, av[0]);
            exit(EXIT_FAILURE);
        }
    }
    for(int x = optind; x < ac; x++) {

        if(data.opt.team) {

            fprintf(stderr, RED"Invalid argument: %s\n"RESET, av[x]);
            printf(USAGE, av[0]);
            exit(EXIT_FAILURE);
        }
        if(!av[x][0] || strlen(av[x]) > NAME_SIZE - 1) {

            fprintf(stderr, RED"Invalid team name: %s\n"RESET, av[x]);
            printf(USAGE, av[0]);
            exit(EXIT_FAILURE);
        }
        data.opt.team = av[x];
    }
    if(data.opt.max_players * PLAYER_VOLUME * PLAYER_VOLUME > data.opt.width * data.opt.height) {

        fprintf(stderr, RED"Board would be too small.\n"RESET);
        printf(USAGE, av[0]);
        exit(EXIT_FAILURE);
    }
    if(data.opt.team) return;
    fprintf(stderr, RED"Missing argument: TEAM\n"RESET);
    printf(USAGE, av[0]);
    exit(EXIT_FAILURE);
}
