#include "../include/header.h"

extern t_lemipc data;

static byte failure(const char* const msg) {

    if(data.first) data.abort = YES;
    data.code = errno;
    perror(msg);
    return EXIT_FAILURE;
}

static byte file_init(const char* const path) {

    FILE* file = fopen(path, "r");
    if(!file) {

        file = fopen(path, "w");
        if(!file) return EXIT_FAILURE;
        data.first = YES;
    }
    fclose(file);
    return EXIT_SUCCESS;
}

static byte msg_init(int* const msgid, const ubyte idx) {

    char path[32] = {0};
    sprintf(path, MSG_PATH, idx);
    if(file_init(path) != EXIT_SUCCESS) return EXIT_FAILURE;

    key_t key = ftok(path, IPC_ID);
    if(key == -1) return EXIT_FAILURE;

    *msgid = msgget(key, IPC_CREAT | 0666);
    return (*msgid == -1) ? EXIT_FAILURE : EXIT_SUCCESS;
}

static void new_sem(t_sembuf* const lock,
                    t_sembuf* const unlock,
                    const ushort semnum) {

    lock->sem_num = semnum;
    lock->sem_op = -1;
    lock->sem_flg = 0;

    unlock->sem_num = semnum;
    unlock->sem_op = 1;
    unlock->sem_flg = 0;
}

static byte sem_init() {

    data.sem = malloc(SEM_SIZE);
    if(!data.sem) return EXIT_FAILURE;

    memset(data.sem, 0, SEM_SIZE);
    data.sem->timeout.tv_sec = 8;

    key_t key = ftok(SEM_PATH, IPC_ID);
    if(key == -1) return EXIT_FAILURE;
    if(data.first) {

        data.semid = semget(key, SEM_COUNT, IPC_CREAT | 0666);
        if(data.semid == -1) return EXIT_FAILURE;

        t_semun semun = {0};
        ushort values[SEM_COUNT];

        memset(values, 1, sizeof(values));
        semun.array = values;

        if(semctl(data.semid, 0, SETALL, semun) == -1)
            return EXIT_FAILURE;
    } else {
        for(ubyte x = 0; x < 8; x++) {

            data.semid = semget(key, SEM_COUNT, 0666);
            if(data.semid == -1) sleep(1);
            else break;
        }
        if(data.semid == -1) return EXIT_FAILURE;
    }
    new_sem(&data.sem->board_lock,
            &data.sem->board_unlock,
            SEM_BOARD);

    new_sem(&data.sem->teams_lock,
            &data.sem->teams_unlock,
            SEM_TEAMS);

    new_sem(&data.sem->party_lock,
            &data.sem->party_unlock,
            SEM_PARTY);

    new_sem(&data.sem->gui_lock,
            &data.sem->gui_unlock,
            SEM_GUI);

    return EXIT_SUCCESS;
}

static byte shm_init() {

    key_t key = ftok(SHM_PATH, IPC_ID);
    if(key == -1)  return EXIT_FAILURE;
    if(data.first) {

        data.shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
        if(data.shmid == -1) return EXIT_FAILURE;
    } else {
        for(ubyte x = 0; x < 8; x++) {

            data.shmid = shmget(key, SHM_SIZE, 0666);
            if(data.shmid == -1) sleep(1);
            else break;
        }
        if(data.shmid == -1) return EXIT_FAILURE;
    }
    data.shm = shmat(data.shmid, NULL, 0);
    return (data.shm == (void*)-1) ? EXIT_FAILURE : EXIT_SUCCESS;
}

byte init() {

    if(file_init(SHM_PATH) != EXIT_SUCCESS) return EXIT_FAILURE;
    if(data.first) {

        if(file_init(SEM_PATH) != EXIT_SUCCESS)
            return failure("init: file_init");
    } else {
        bool ok = NO;
        for(ubyte x = 0; x < 8; x++) {

            if(access(SEM_PATH, F_OK) || access(SHM_PATH, F_OK)) {
                sleep(1);
                continue;
            }
            ok = YES;
            break;
        }
        data.code = ok ? EXIT_SUCCESS : EXIT_FAILURE;
        if(!ok) return data.code;
    }
    if(shm_init() != EXIT_SUCCESS) return failure("init: shm_init");
    if(sem_init() != EXIT_SUCCESS) return failure("init: sem_init");
    if(data.first) {

        t_team* team;
        bool ok = YES;
        const int colors[MAX_TEAMS] = {

            RGB_RED_, RGB_GREEN_, RGB_BLUE_,
            RGB_CYAN_, RGB_YELLOW_, RGB_MAGENTA_
        };
        if(semtimedop(data.semid, &data.sem->party_lock,
                      1, &data.sem->timeout))
            return failure("init: semtimedop");

        memset(data.shm, 0, SHM_SIZE);
        for(ubyte x = 0; x < MAX_TEAMS && ok; x++) {

            team = &data.shm->teams[x];
            team->color = colors[x];
            if(msg_init(&team->msgid, x) == EXIT_SUCCESS) continue;
            ok = NO;
            break;
        }
        if(ok) {
            t_player* player;
            for(ushort x = 0; x < BOARD_WIDTH; x++) {
                for(ushort y = 0; y < BOARD_HEIGHT; y++) {

                    player = &data.shm->board[x][y];
                    player->x = x;
                    player->y = y;
                }
            }
            data.shm->initialized = YES;
        }
        if(semtimedop(data.semid, &data.sem->party_unlock,
                      1, &data.sem->timeout))
            return failure("init: semtimedop");

        return ok ? EXIT_SUCCESS : EXIT_FAILURE;
    }
    bool ok = NO;
    for(ubyte x = 0; x < 8; x++) {

        if(semtimedop(data.semid, &data.sem->party_lock,
                      1, &data.sem->timeout))
            return failure("init: semtimedop");

        if(data.shm->initialized) ok = YES;

        if(semtimedop(data.semid, &data.sem->party_unlock,
                      1, &data.sem->timeout))
            return failure("init: semtimedop");

        if(ok) break;
        else sleep(1);
    };
    data.code = ok ? EXIT_SUCCESS : EXIT_FAILURE;
    return data.code;
}
