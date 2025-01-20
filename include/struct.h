#ifndef STRUCT_H
#define STRUCT_H

typedef struct opt {
    char* team;
    bool gui;
    bool quiet;
} t_opt;

typedef struct s_team {
    char name[NAME_SIZE];
    int color;
    int msgid;
    pid_t ids[MAX_PLAYERS];
    ubyte players_count;
} t_team;

typedef struct s_player {
    pid_t id;
    int color;
    ushort x;
    ushort y;
    byte directions[4][2];
} t_player;

typedef struct s_shm {
    t_player board[BOARD_WIDTH][BOARD_HEIGHT];
    t_team teams[MAX_TEAMS];
    ubyte teams_count;
    ubyte players_count;
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
    t_sembuf board_lock;
    t_sembuf board_unlock;
    t_sembuf teams_lock;
    t_sembuf teams_unlock;
    t_sembuf party_lock;
    t_sembuf party_unlock;
    t_sembuf gui_lock;
    t_sembuf gui_unlock;
    t_timespec timeout;
} t_sem;

typedef struct position {
    ushort x;
    ushort y;
    bool set;
} t_position;

typedef struct msg {
    long type;
    char text[IPC_MSG_SIZE];
} t_msg;

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
    pid_t subs[MAX_SUBS];
    ushort subs_count;
} t_mlx;

typedef struct s_lemipc {
    t_opt opt;
    int shmid;
    int semid;
    t_shm* shm;
    t_sem* sem;
    t_mlx* wm;
    bool first;
    bool joined;
    bool abort;
    bool is_sub;
    byte code;
    t_player self;
    t_player nearest;
    t_player target;
} t_lemipc;

#define OPT_SIZE sizeof(t_opt)
#define TEAM_SIZE sizeof(t_team)
#define PLAYER_SIZE sizeof(t_player)
#define SHM_SIZE sizeof(t_shm)
#define SEM_SIZE sizeof(t_sem)
#define MSG_SIZE sizeof(t_msg)
#define IMG_SIZE sizeof(t_img)
#define MLX_SIZE sizeof(t_mlx)
#define LEMIPC_SIZE sizeof(t_lemipc)

#endif
