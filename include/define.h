#ifndef DEFINE_H
#define DEFINE_H

#define TO_STRING_(x) #x
#define TO_STRING(x) TO_STRING_(x)

#define IPC_ID 42
#define SHM_PATH "/tmp/lemipc-shm"
#define SEM_PATH "/tmp/lemipc-sem"
#define MSG_PATH "/tmp/lemipc-msg-%d"
#define CMD "./lemipc %s"

#define SEM_COUNT 7
#define SEM_INIT 1
#define SEM_BOARD 2
#define SEM_TEAMS 3
#define SEM_PLAYERS_COUNT 4
#define SEM_GUI 5
#define SEM_PARTY 6

#define PIXEL_SIZE 10
#define PLAYER_VOLUME 2

#define MAX_WIDTH 132
#define MAX_HEIGHT 88
#define MAX_TEAMS 6
#define MAX_PLAYERS 36
#define MAX_SUBS 1024

#define MSG_SIZE 256
#define NAME_SIZE 32
#define NAME_SIZE_ 31
#define COLOR_SIZE 16

#define FLED_DISTANCE 8

#define RESET "\033[0m"
#define RED "\033[31m"
#define GREEN "\033[32m"

#define GUI_HEADER 24 * PIXEL_SIZE
#define GUI_FONT_HEAD "-*-clean-*-*-*-*-24-240-*-*-*-*-*-*"
#define GUI_FONT "-*-clean-*-*-*-*-20-200-*-*-*-*-*-*"

#define RGB_RED 0x00CC8888
#define RGB_RED_ 0x00CC4444
#define RGB_GREEN 0x0088CC88
#define RGB_GREEN_ 0x0044CC44
#define RGB_BLUE 0x008888CC
#define RGB_BLUE_ 0x004444CC
#define RGB_CYAN 0x0088CCCC
#define RGB_CYAN_ 0x0044CCCC
#define RGB_YELLOW 0x00CCCC88
#define RGB_YELLOW_ 0x00CCCC44
#define RGB_MAGENTA 0x00CC88CC
#define RGB_MAGENTA_ 0x00CC44CC

#define RGB_BLACK 0x00000000
#define RGB_GRAY_D 0x00111111
#define RGB_GRAY 0x00777777
#define RGB_GRAY_L 0x00EEEEEE
#define RGB_WHITE 0x00FFFFFF

#define RGB_HEADER 0x00333333
#define RGB_HEADER_ 0x00222222

#define USAGE "\n"\
"%s [TEAM] " GREEN "[OPTIONS]\n" RESET \
"\n" \
"TEAM: The name of the team to join. (1 - " TO_STRING(NAME_SIZE_) " characters)\n" \
"OPTIONS:\n" \
GREEN "     -g, --gui" RESET "       Launch a GUI client.\n" \
GREEN "     -q, --quiet" RESET "     Quiet player.\n" \
GREEN "     -h, --help" RESET "      Display this help.\n" \
"\n"

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1
#define EXIT_GAMEOVER 42

#define YES 1
#define NO 0

typedef char byte;
typedef unsigned char ubyte;
typedef unsigned char bool;
typedef unsigned short ushort;

typedef struct option t_option;
typedef struct semid_ds t_semid_ds;
typedef struct sembuf t_sembuf;
typedef struct winsize t_winsize;
typedef struct timespec t_timespec;
typedef struct timeval t_timeval;

#endif
