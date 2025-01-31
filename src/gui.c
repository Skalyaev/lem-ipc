#include "../include/header.h"

extern t_lemipc data;

void new_pixel(t_img* const screen,
               const int x, const int y,
               const int color) {

    const int bpp = screen->bpp / 8;
    const int xsize = x * bpp;
    const int ysize = y * screen->line_size;

    *(uint*)(screen->addr + ysize + xsize) = color;
}

static void party_info(const bool* const started,
                       const bool* const paused,
                       const bool* const over,
                       const t_timeval* const start,
                       const t_timeval* const pause,
                       const t_timeval* const end,
                       const int x, const int y) {

    mlx_set_font(data.wm->mlx, data.wm->win, GUI_FONT);
    mlx_string_put(data.wm->mlx, data.wm->win, x, y, RGB_WHITE, "Duration:");
    char time[32] = {0};
    if(*started) {

        t_timeval now, out;
        gettimeofday(&now, NULL);

        if(*over) timersub(end, start, &out);
        else {
            t_timeval tmp;
            timersub(&now, start, &out);
            if(*paused) {

                timersub(&now, pause, &tmp);
                timersub(&out, &tmp, &out);
            }
        }
        if(out.tv_sec < 0) out.tv_sec = 0;
        if(out.tv_usec < 0) out.tv_usec = 0;

        sprintf(time, "%02ld:%02ld", out.tv_sec / 60, out.tv_sec % 60);
    }
    else strcpy(time, "00:00");
    mlx_set_font(data.wm->mlx, data.wm->win, GUI_FONT_HEAD);
    mlx_string_put(data.wm->mlx, data.wm->win, x + 132, y + 2, RGB_WHITE, time);

    char cmd[32] = {0};
    if(*paused) strcpy(cmd, "PLAY");
    else strcpy(cmd, "PAUSE");
    mlx_string_put(data.wm->mlx, data.wm->win, x, y + 42, RGB_WHITE, cmd);

    int color = *started && !*over ? RGB_WHITE : RGB_GRAY;
    strcpy(cmd, "STOP");
    mlx_string_put(data.wm->mlx, data.wm->win, x + 140, y + 42, color, cmd);
}

static void team_info(const int x, const int y, const t_team* const team) {

    char name[14] = {0};
    char player_count[32] = {0};

    if(strlen(team->name) > 10) {

        strncpy(name, team->name, 10);
        strcat(name, "...");
    }
    else strcpy(name, team->name);
    sprintf(player_count, "%d", team->players_count);

    mlx_set_font(data.wm->mlx, data.wm->win, GUI_FONT);
    mlx_string_put(data.wm->mlx, data.wm->win, x, y, RGB_WHITE, name);

    mlx_set_font(data.wm->mlx, data.wm->win, GUI_FONT_HEAD);
    mlx_string_put(data.wm->mlx, data.wm->win, x, y + 32, RGB_WHITE, player_count);
    mlx_string_put(data.wm->mlx, data.wm->win, x + GUI_HEADER - 48, y + 32, RGB_WHITE, "+");
}

static byte draw_text() {

    static t_team teams[MAX_TEAMS];
    static const size_t teams_size = sizeof(teams);

    mlx_set_font(data.wm->mlx, data.wm->win, GUI_FONT_HEAD);
    mlx_string_put(data.wm->mlx, data.wm->win, BOARD_SCREEN_WIDTH + 76, 32,
                   RGB_WHITE, "LEM IPC");

    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw_text: semtimedop");
        return EXIT_FAILURE;
    }
    memcpy(teams, data.shm->teams, teams_size);
    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw_text: semtimedop");
        return EXIT_FAILURE;
    }
    for(ubyte x = 0; x < MAX_TEAMS; x++)
        team_info(BOARD_SCREEN_WIDTH + 20, 76 + x * 80, &teams[x]);

    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw_text: semtimedop");
        return EXIT_FAILURE;
    }
    bool started = data.shm->started;
    bool paused = data.shm->paused;
    bool over = data.shm->over;

    t_timeval start = data.shm->start;
    t_timeval pause = data.shm->pause;
    t_timeval end = data.shm->end;

    if(semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw_text: semtimedop");
        return EXIT_FAILURE;
    }
    party_info(&started, &paused, &over, &start, &pause, &end,
               BOARD_SCREEN_WIDTH + 20, 88 + MAX_TEAMS * 80);

    return EXIT_SUCCESS;
}

static void draw_header_bg() {

    for(ushort y = 0; y < BOARD_SCREEN_HEIGHT; y++) {
        for(ushort x = BOARD_SCREEN_WIDTH; x < SCREEN_WIDTH; x++) {

            if(x < BOARD_SCREEN_WIDTH + 8)
                new_pixel(&data.wm->screen, x, y, RGB_HEADER_);

            else if(y > 48 && y < 128) {

                if(y > 120) new_pixel(&data.wm->screen, x, y, RGB_RED);
                else new_pixel(&data.wm->screen, x, y, RGB_RED_);
            }
            else if(y > 128 && y < 208) {

                if(y > 200) new_pixel(&data.wm->screen, x, y, RGB_GREEN);
                else new_pixel(&data.wm->screen, x, y, RGB_GREEN_);
            }
            else if(y > 208 && y < 288) {

                if(y > 280) new_pixel(&data.wm->screen, x, y, RGB_BLUE);
                else new_pixel(&data.wm->screen, x, y, RGB_BLUE_);
            }
            else if(y > 288 && y < 368) {

                if(y > 360) new_pixel(&data.wm->screen, x, y, RGB_CYAN);
                else new_pixel(&data.wm->screen, x, y, RGB_CYAN_);
            }
            else if(y > 368 && y < 448) {

                if(y > 440) new_pixel(&data.wm->screen, x, y, RGB_YELLOW);
                else new_pixel(&data.wm->screen, x, y, RGB_YELLOW_);
            }
            else if(y > 448 && y < 528) {

                if(y > 520) new_pixel(&data.wm->screen, x, y, RGB_MAGENTA);
                else new_pixel(&data.wm->screen, x, y, RGB_MAGENTA_);
            }
            else new_pixel(&data.wm->screen, x, y, RGB_HEADER);
        }
    }
}

static int board_color(const t_player* const player) {

    if(!player->id) return RGB_GRAY_D;
    return player->color;
}

static byte draw_board() {

    static t_player board[BOARD_WIDTH][BOARD_HEIGHT];
    static const size_t board_size = sizeof(board);

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw_board: semtimedop");
        return EXIT_FAILURE;
    }
    memcpy(board, data.shm->board, board_size);
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw_board: semtimedop");
        return EXIT_FAILURE;
    }
    for(ushort x = 0; x < BOARD_WIDTH; x++)
        for(ushort y = 0; y < BOARD_HEIGHT; y++)

            for(ubyte xx = 0; xx < BOARD_PLAYER_SIZE; xx++)
                for(ubyte yy = 0; yy < BOARD_PLAYER_SIZE; yy++)

                    new_pixel(&data.wm->screen,
                              x * BOARD_PLAYER_SIZE + xx,
                              y * BOARD_PLAYER_SIZE + yy,
                              board_color(&board[x][y]));
    return EXIT_SUCCESS;
}

static int loop_hook() {

    if(draw_board() != EXIT_SUCCESS) exit(bye());
    draw_header_bg();
    mlx_put_image_to_window(data.wm->mlx, data.wm->win,
                            data.wm->screen.img, 0, 0);

    if(draw_text() != EXIT_SUCCESS) exit(bye());
    usleep(100000);
    return EXIT_SUCCESS;
}

static int key_hook(const int key, void* const param) {

    (void)param;
    if(key == 65307) exit(bye());
    return EXIT_SUCCESS;
}

static int mouse_hook(const int button, int x, int y, void* const param) {
    (void)button;
    (void)param;
    mlx_mouse_get_pos(data.wm->mlx, data.wm->win, &x, &y);
    if(x <= BOARD_SCREEN_WIDTH + 8) return EXIT_SUCCESS;
    if(y > 48 && y < 528) {

        if(y > 48 && y <= 128 && add_player(0) != EXIT_SUCCESS) exit(bye());
        else if(y > 128 && y <= 208 && add_player(1) != EXIT_SUCCESS) exit(bye());
        else if(y > 208 && y <= 288 && add_player(2) != EXIT_SUCCESS) exit(bye());
        else if(y > 288 && y <= 368 && add_player(3) != EXIT_SUCCESS) exit(bye());
        else if(y > 368 && y <= 448 && add_player(4) != EXIT_SUCCESS) exit(bye());
        else if(y > 448 && y <= 528 && add_player(5) != EXIT_SUCCESS) exit(bye());
    }
    else if(y > 588 && y < 624) {

        if(x < BOARD_SCREEN_WIDTH + 100
                && pause_game() != EXIT_SUCCESS) exit(bye());

        else if(x > BOARD_SCREEN_WIDTH + 152 && x < BOARD_SCREEN_WIDTH + 228
                && stop_game() != EXIT_SUCCESS) exit(bye());
    }
    return EXIT_SUCCESS;
}

byte draw() {

    data.wm = malloc(MLX_SIZE);
    if(!data.wm) {

        data.code = errno;
        perror("draw: malloc");
        return bye();
    }
    memset(data.wm, 0, MLX_SIZE);

    bool gui = YES;
    if(semtimedop(data.semid, &data.sem->gui_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw: semtimedop");
        return bye();
    }
    if(!data.shm->gui) {
        gui = NO;
        data.shm->gui = YES;
    }
    if(semtimedop(data.semid, &data.sem->gui_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("draw: semtimedop");
        return bye();
    }
    if(gui) return bye();

    data.wm->mlx = mlx_init();
    data.wm->win = mlx_new_window(data.wm->mlx, SCREEN_WIDTH, SCREEN_HEIGHT,
                                  "LemIPC");

    t_img* const screen = &data.wm->screen;
    screen->img = mlx_new_image(data.wm->mlx, SCREEN_WIDTH, SCREEN_HEIGHT);
    screen->addr = mlx_get_data_addr(screen->img, &screen->bpp,
                                     &screen->line_size, &screen->endian);

    mlx_mouse_hook(data.wm->win, mouse_hook, NULL);
    mlx_key_hook(data.wm->win, key_hook, NULL);
    mlx_loop_hook(data.wm->mlx, loop_hook, NULL);
    mlx_loop(data.wm->mlx);
    return bye();
}
