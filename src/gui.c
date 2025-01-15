#include "../include/header.h"

extern t_lemipc data;

void new_pixel(t_img* const screen,
               const int x, const int y, const int color) {

    const int bpp = screen->bpp / 8;
    const int xsize = x * bpp;
    const int ysize = y * screen->line_size;

    *(uint*)(screen->addr + ysize + xsize) = color;
}

static void team_info(const int x, const int y, const ushort idx) {

    char name[14] = {0};
    char player_count[32] = {0};

    t_team* const team = &data.shm->teams[idx];
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

static void party_info(const bool* const started,
                       const bool* const paused,
                       const bool* const over,
                       const t_timeval* const start,
                       const t_timeval* const pause,
                       const t_timeval* const end,
                       const int x, const int y) {

    static const byte yoff = 42;

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
    mlx_string_put(data.wm->mlx, data.wm->win, x, y + yoff, RGB_WHITE, cmd);

    strcpy(cmd, "STOP");
    int color = *started && !*over ? RGB_WHITE : RGB_GRAY;
    mlx_string_put(data.wm->mlx, data.wm->win, x + 140, y + yoff, color, cmd);
}

static byte draw_text(const int* const xoffset) {

    mlx_set_font(data.wm->mlx, data.wm->win, GUI_FONT_HEAD);
    mlx_string_put(data.wm->mlx, data.wm->win,
                   *xoffset + 76, 32, RGB_WHITE, "LEM IPC");

    if(semtimedop(data.semid, &data.sem->teams_lock, 1, &data.sem->timeout)) {

        perror("draw_text: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    for(ushort x = 0; x < data.shm->max_teams; x++)
        team_info(*xoffset + 20, 76 + (x * 80), x);

    if(semtimedop(data.semid, &data.sem->teams_unlock, 1, &data.sem->timeout)) {

        perror("draw_text: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    if(semtimedop(data.semid, &data.sem->party_lock, 1, &data.sem->timeout)) {

        perror("draw_text: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    bool started = data.shm->started;
    bool paused = data.shm->paused;
    bool over = data.shm->over;

    t_timeval start = data.shm->start;
    t_timeval pause = data.shm->pause;
    t_timeval end = data.shm->end;

    if(semtimedop(data.semid, &data.sem->party_unlock, 1, &data.sem->timeout)) {

        perror("draw_text: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    party_info(&started, &paused, &over, &start, &pause, &end,
               *xoffset + 20, 88 + (data.shm->max_teams * 80));
    return EXIT_SUCCESS;
}

static void draw_header(const int* const width,
                        const int* const height) {

    for(ushort y = 0; y < *height; y++) {
        for(ushort x = *width; x < *width + GUI_HEADER; x++) {

            if(x < *width + 8)
                new_pixel(&data.wm->screen, x, y, RGB_HEADER_);

            else if(y > 48 && y < 128) {

                if(y > 120) new_pixel(&data.wm->screen, x, y, RGB_RED);
                else new_pixel(&data.wm->screen, x, y, RGB_RED_);
            }
            else if(y > 128 && y < 208) {

                if(y > 200) new_pixel(&data.wm->screen, x, y, RGB_GREEN);
                else new_pixel(&data.wm->screen, x, y, RGB_GREEN_);
            }
            else if(y > 208 && y < 288 && data.shm->max_teams > 2) {

                if(y > 280) new_pixel(&data.wm->screen, x, y, RGB_BLUE);
                else new_pixel(&data.wm->screen, x, y, RGB_BLUE_);
            }
            else if(y > 288 && y < 368 && data.shm->max_teams > 5) {

                if(y > 360) new_pixel(&data.wm->screen, x, y, RGB_CYAN);
                else new_pixel(&data.wm->screen, x, y, RGB_CYAN_);
            }
            else if(y > 368 && y < 448 && data.shm->max_teams > 3) {

                if(y > 440) new_pixel(&data.wm->screen, x, y, RGB_YELLOW);
                else new_pixel(&data.wm->screen, x, y, RGB_YELLOW_);
            }
            else if(y > 448 && y < 528 && data.shm->max_teams > 4) {

                if(y > 520) new_pixel(&data.wm->screen, x, y, RGB_MAGENTA);
                else new_pixel(&data.wm->screen, x, y, RGB_MAGENTA_);
            }
            else new_pixel(&data.wm->screen, x, y, RGB_HEADER);
        }
    }
}

static byte draw_map() {

    static const ushort pixel_size = PIXEL_SIZE * PLAYER_VOLUME;
    const ushort board_width = data.shm->width / PLAYER_VOLUME;
    const ushort board_height = data.shm->height / PLAYER_VOLUME;

    int map[board_width][board_height];
    memset(map, 0, sizeof(map));

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        perror("draw_map: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    t_player* player;
    for(ushort x = 0; x < board_width; x++) {
        for(ushort y = 0; y < board_height; y++) {

            player = &data.shm->board[x][y];
            if(player->color) map[x][y] = player->color;
            else map[x][y] = RGB_GRAY_D;
        }
    }
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {

        perror("draw_map: semtimedop");
        data.code = errno;
        return EXIT_FAILURE;
    }
    for(ushort w = 0; w < board_width; w++)
        for(ushort x = 0; x < board_height; x++)

            for(ushort y = 0; y < pixel_size; y++)
                for(ushort z = 0; z < pixel_size; z++)
                    new_pixel(&data.wm->screen,
                              w * pixel_size + y,
                              x * pixel_size + z,
                              map[w][x]);
    return EXIT_SUCCESS;
}

static int loop_hook() {

    const int width = data.shm->width * PIXEL_SIZE;
    const int height = data.shm->height * PIXEL_SIZE;

    if(draw_map() != EXIT_SUCCESS) exit(bye());
    draw_header(&width, &height);

    mlx_put_image_to_window(data.wm->mlx, data.wm->win,
                            data.wm->screen.img, 0, 0);

    if(draw_text(&width) != EXIT_SUCCESS) exit(bye());
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
    const int width = data.shm->width * PIXEL_SIZE;

    mlx_mouse_get_pos(data.wm->mlx, data.wm->win, &x, &y);
    if(x > width + 8) {

        if(y > 48 && y < 528) {

            if(y > 48 && y <= 128 && add_player(0) != EXIT_SUCCESS) exit(bye());
            else if(y > 128 && y <= 208 && add_player(1) != EXIT_SUCCESS) exit(bye());
            else if(y > 208 && y <= 288 && add_player(2) != EXIT_SUCCESS) exit(bye());
            else if(y > 288 && y <= 368 && add_player(3) != EXIT_SUCCESS) exit(bye());
            else if(y > 368 && y <= 448 && add_player(4) != EXIT_SUCCESS) exit(bye());
            else if(y > 448 && y <= 528 && add_player(5) != EXIT_SUCCESS) exit(bye());
        }
        else if(y > 588 && y < 624) {

            if(x < width + 100 && pause_game() != EXIT_SUCCESS) exit(bye());
            else if(x > width + 152 && x < width + 228
                    && stop_game() != EXIT_SUCCESS) exit(bye());
        }
    }
    return EXIT_SUCCESS;
}

byte draw() {

    bool gui = YES;
    if(semtimedop(data.semid, &data.sem->gui_lock, 1, &data.sem->timeout)) {

        perror("draw: semtimedop");
        data.code = errno;
        return bye();
    }
    if(!data.shm->gui) {
        gui = NO;
        data.shm->gui = YES;
    }
    if(semtimedop(data.semid, &data.sem->gui_unlock, 1, &data.sem->timeout)) {

        perror("draw: semtimedop");
        data.code = errno;
        return bye();
    }
    if(gui) return EXIT_SUCCESS;

    const int width = data.shm->width * PIXEL_SIZE + GUI_HEADER;
    const int height = data.shm->height * PIXEL_SIZE;

    t_img* screen = &data.wm->screen;
    screen->img = mlx_new_image(data.wm->mlx, width, height);
    screen->addr = mlx_get_data_addr(screen->img, &screen->bpp,
                                     &screen->line_size, &screen->endian);

    data.wm->win = mlx_new_window(data.wm->mlx, width, height, "LemIPC");

    mlx_mouse_hook(data.wm->win, mouse_hook, NULL);
    mlx_key_hook(data.wm->win, key_hook, NULL);
    mlx_loop_hook(data.wm->mlx, loop_hook, NULL);

    mlx_loop(data.wm->mlx);
    return bye();
}
