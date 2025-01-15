#ifndef STRUCT_H
#define STRUCT_H

typedef struct opt {
    char* team;
    ushort width;
    ushort height;
    ubyte max_teams;
    ubyte max_players;
    bool gui;
    bool quiet;
} t_opt;

typedef struct s_team {
    int msgid;
    ubyte ids[MAX_PLAYERS];
    char name[NAME_SIZE];
    int color;
    ubyte players_count;
} t_team;

typedef struct s_player {
    ubyte id;
    ubyte x;
    ubyte y;
    int color;
} t_player;

typedef struct s_shm {
    ubyte width;
    ubyte height;
    ubyte max_teams;
    ubyte max_players;
    t_player board[MAX_WIDTH][MAX_HEIGHT];
    t_team teams[MAX_TEAMS];
    ubyte players_count;
    ubyte teams_count;
    bool initialized;
    bool gui;
    bool started;
    bool paused;
    bool over;
    t_timeval start;
    t_timeval pause;
    t_timeval end;
} t_shm;

typedef union semun {
    int value;
    t_semid_ds* buffer;
    ushort* array;
} t_semun;

typedef struct s_sem {
    t_sembuf init_lock;
    t_sembuf init_unlock;
    t_sembuf board_lock;
    t_sembuf board_unlock;
    t_sembuf teams_lock;
    t_sembuf teams_unlock;
    t_sembuf players_count_lock;
    t_sembuf players_count_unlock;
    t_sembuf gui_lock;
    t_sembuf gui_unlock;
    t_sembuf party_lock;
    t_sembuf party_unlock;
    t_timespec timeout;
} t_sem;

typedef struct s_img {
    void* img;
    char* addr;
    int bpp;
    int line_size;
    int endian;
} t_img;

typedef struct s_mlx {
    void* mlx;
    void* win;
    t_img screen;
} t_mlx;

typedef struct msg {
    long type;
    char text[MSG_SIZE];
} t_msg;

typedef struct s_lemipc {
    t_opt opt;
    t_player* self;
    t_player target;
    t_player nearest;
    t_shm* shm;
    t_sem* sem;
    t_mlx* wm;
    int shmid;
    int semid;
    byte code;
    bool first;
    bool abort;
    bool joined;
    pid_t subs[MAX_SUBS];
    ushort subs_count;
    bool is_sub;
    byte direction[2];
} t_lemipc;

#endif
