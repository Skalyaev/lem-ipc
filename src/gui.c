#include "../include/header.h"

extern t_lemipc data;

void new_pixel(t_img* const screen,
               const int x, const int y, const int color) {

    const int bpp = screen->bpp / 8;
    const int xsize = x * bpp;
    const int ysize = y * screen->line_size;

    char* dst = screen->addr + ysize + xsize;
    *(uint*)dst = color;
}

static byte put_pixels(t_img* const screen) {

    int map[data.shm->width][data.shm->height];
    memset(map, 0, sizeof(map));

    if(semtimedop(data.semid, &data.sem->board_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("semtimedop");
        return EXIT_FAILURE;
    }
    t_player* player;
    for(ushort x = 0; x < data.shm->width; x++) {
        for(ushort y = 0; y < data.shm->height; y++) {

            player = &data.shm->board[x][y];
            if(player->team) map[x][y] = player->team->color;
            else map[x][y] = RGB_GRAY;
        }
    }
    if(semtimedop(data.semid, &data.sem->board_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("semtimedop");
        return EXIT_FAILURE;
    }
    for(ushort w = 0; w < data.shm->width; w++)
        for(ushort x = 0; x < data.shm->height; x++)

            for(ushort y = 0; y < PIXEL_SIZE; y++)
                for(ushort z = 0; z < PIXEL_SIZE; z++)
                    new_pixel(screen,
                              w * PIXEL_SIZE + y,
                              x * PIXEL_SIZE + z,
                              map[w][x]);
    return EXIT_SUCCESS;
}

static int loop() {

    put_pixels(&data.wm->screen);
    mlx_put_image_to_window(data.wm->mlx, data.wm->win,
                            data.wm->screen.img, 0, 0);
    usleep(100000);
    return EXIT_SUCCESS;
}

byte draw() {

    if(semtimedop(data.semid, &data.sem->gui_lock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("semtimedop");
        return bye();
    }
    data.shm->gui = YES;
    if(semtimedop(data.semid, &data.sem->gui_unlock, 1, &data.sem->timeout)) {

        data.code = errno;
        perror("semtimedop");
        return bye();
    }
    const size_t wm_size = sizeof(t_mlx);
    data.wm = malloc(wm_size);
    if(!data.wm) {

        perror("malloc");
        data.code = errno;
        return bye();
    }
    memset(data.wm, 0, wm_size);

    const int width = data.shm->width * PIXEL_SIZE;
    const int height = data.shm->height * PIXEL_SIZE;

    data.wm->mlx = mlx_init();
    data.wm->win = mlx_new_window(data.wm->mlx, width, height, "LemIPC");

    t_img* screen = &data.wm->screen;
    screen->img = mlx_new_image(data.wm->mlx, width, height);
    screen->addr = mlx_get_data_addr(screen->img, &screen->bpp,
                                     &screen->line_size, &screen->endian);

    mlx_loop_hook(data.wm->mlx, loop, NULL);
    mlx_loop(data.wm->mlx);
    return bye();
}
