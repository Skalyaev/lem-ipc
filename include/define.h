#ifndef DEFINE_H
#define DEFINE_H

#define TO_STRING_(x) #x
#define TO_STRING(x) TO_STRING_(x)

#define IPC_ID 42
#define SHM_PATH "/tmp/lemipc-shm"
#define SEM_PATH "/tmp/lemipc-sem"

#define SEM_COUNT 6
#define SEM_INIT 1
#define SEM_BOARD 2
#define SEM_TEAMS 3
#define SEM_PLAYERS_COUNT 4
#define SEM_GUI 5


#define PIXEL_SIZE 10
#define PLAYER_VOLUME 2

#define MAX_WIDTH 128
#define MAX_HEIGHT 78
#define MAX_TEAMS 6
#define MAX_PLAYERS 32

#define NAME_SIZE 32
#define NAME_SIZE_ 31
#define COLOR_SIZE 16

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"

#define RGB_RED 0x00FF7777
#define RGB_GREEN 0x0077FF77
#define RGB_YELLOW 0x00FFFF77
#define RGB_MAGENTA 0x00FF77FF
#define RGB_BLUE 0x007777FF
#define RGB_CYAN 0x0077FFFF
#define RGB_BLACK 0x00000000
#define RGB_GRAY 0x00111111
#define RGB_WHITE 0x00FFFFFF

#define USAGE "\n"\
"%s TEAM " GREEN "[OPTIONS]\n" RESET \
"\n" \
"TEAM: The name of the team to join. (1 - " TO_STRING(NAME_SIZE_) " characters)\n" \
"OPTIONS:\n" \
GREEN "     -W, --width " RESET "(2 - " TO_STRING(MAX_WIDTH) ")"\
      "     The width of the board. Default to " TO_STRING(MAX_WIDTH) ".\n" \
GREEN "     -H, --height " RESET "(1 - " TO_STRING(MAX_HEIGHT) ")" \
      "          The height of the board. Default to " TO_STRING(MAX_HEIGHT) ".\n"\
GREEN "     -t, --max-teams " RESET "(2 - " TO_STRING(MAX_TEAMS) ")" \
      "        The maximum number of teams. Default to " TO_STRING(MAX_TEAMS) ".\n" \
GREEN "     -p, --max-players " RESET "(2 - " TO_STRING(MAX_PLAYERS) ")" \
      "     The maximum number of players per team. Default to " TO_STRING(MAX_PLAYERS) ".\n" \
GREEN "     -g, --gui" RESET \
      "                     Launch a GUI client.\n" \
GREEN "     -h, --help" RESET \
      "                     Display this help.\n" \
"\n"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define YES 1
#define NO 0

typedef unsigned char byte;
typedef unsigned char ubyte;
typedef unsigned char bool;
typedef unsigned short ushort;

typedef struct option t_option;
typedef struct semid_ds t_semid_ds;
typedef struct sembuf t_sembuf;
typedef struct timespec t_timespec;
typedef struct winsize t_winsize;

#endif
