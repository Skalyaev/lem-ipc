#include "../include/header.h"

extern t_lemipc data;

static byte failure(const char* const msg) {

    perror(msg);
    data.code = errno;
    if(data.first) data.abort = YES;
    return EXIT_FAILURE;
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

    const size_t sem_size = sizeof(t_sem);
    data.sem = malloc(sem_size);
    if(!data.sem) return failure("sem_init: malloc");

    memset(data.sem, 0, sem_size);
    data.sem->timeout.tv_sec = 8;

    key_t key = ftok(SEM_PATH, IPC_ID);
    if(key == -1) return failure("sem_init: ftok");
    if(data.first) {

        data.semid = semget(key, SEM_COUNT, IPC_CREAT | IPC_EXCL | 0666);
        if(data.semid == -1) return failure("sem_init: semget");

        ushort values[SEM_COUNT];
        memset(values, 1, sizeof(values));

        t_semun semun = {0};
        semun.array = values;

        if(semctl(data.semid, 0, SETALL, semun) == -1)
            return failure("sem_init: semctl");
    } else {
        for(ubyte x = 0; x < 8; x++) {

            data.semid = semget(key, SEM_COUNT, 0666);
            if(data.semid == -1) sleep(1);
            else break;
        }
        if(data.semid == -1) return failure("sem_init: semget");
    }
    new_sem(&data.sem->init_lock,
            &data.sem->init_unlock,
            SEM_INIT);

    new_sem(&data.sem->board_lock,
            &data.sem->board_unlock,
            SEM_BOARD);

    new_sem(&data.sem->teams_lock,
            &data.sem->teams_unlock,
            SEM_TEAMS);

    new_sem(&data.sem->players_count_lock,
            &data.sem->players_count_unlock,
            SEM_PLAYERS_COUNT);

    new_sem(&data.sem->gui_lock,
            &data.sem->gui_unlock,
            SEM_GUI);

    new_sem(&data.sem->start_lock,
            &data.sem->start_unlock,
            SEM_START);

    new_sem(&data.sem->party_lock,
            &data.sem->party_unlock,
            SEM_PARTY);

    return EXIT_SUCCESS;
}

static byte shm_init() {

    const size_t shm_size = sizeof(t_shm);

    key_t key = ftok(SHM_PATH, IPC_ID);
    if(key == -1) {

        perror("shm_init: ftok");
        if(data.first) {

            if(remove(SHM_PATH)) perror("shm_init: remove");
            if(remove(SEM_PATH)) perror("shm_init: remove");
        }
        data.code = errno;
        return EXIT_FAILURE;
    }
    if(data.first) {

        data.shmid = shmget(key, shm_size, IPC_CREAT | IPC_EXCL | 0666);
        if(data.shmid == -1) {

            perror("shm_init: shmget");
            if(remove(SHM_PATH)) perror("shm_init: remove");
            if(remove(SEM_PATH)) perror("shm_init: remove");
            data.code = errno;
            return EXIT_FAILURE;
        }
    } else {
        for(ubyte x = 0; x < 8; x++) {

            data.shmid = shmget(key, shm_size, 0666);
            if(data.shmid == -1) sleep(1);
            else break;
        }
    }
    data.shm = shmat(data.shmid, NULL, 0);
    if(data.shm == (void*)-1) {

        perror("shm_init: shmat");
        if(data.first) {

            shmctl(data.shmid, IPC_RMID, NULL);
            if(remove(SHM_PATH)) perror("shm_init: remove");
            if(remove(SEM_PATH)) perror("shm_init: remove");
        }
        data.code = errno;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

static byte file_init(const char* const path) {

    FILE* file = fopen(path, "r");
    if(!file) {

        file = fopen(path, "w");
        if(!file) {

            perror("file_init: fopen");
            data.code = errno;
            return EXIT_FAILURE;
        }
        data.first = YES;
    }
    fclose(file);
    return EXIT_SUCCESS;
}

byte init() {

    if(file_init(SHM_PATH) != EXIT_SUCCESS) return EXIT_FAILURE;
    if(data.first) {

        if(file_init(SEM_PATH) != EXIT_SUCCESS) {

            perror("init: file_init");
            remove(SHM_PATH);
            return EXIT_FAILURE;
        }
    } else {
        bool ok = NO;
        for(ubyte x = 0; x < 8; x++) {

            if(!access(SEM_PATH, F_OK) && !access(SHM_PATH, F_OK)) {
                ok = YES;
                break;
            }
            else sleep(1);
        }
        if(!ok) return EXIT_FAILURE;
    }
    if(shm_init() != EXIT_SUCCESS) return EXIT_FAILURE;
    if(sem_init() != EXIT_SUCCESS) return EXIT_FAILURE;
    if(!data.first) {

        while(YES) {
            if(semtimedop(data.semid, &data.sem->init_lock, 1, &data.sem->timeout))
                return failure("init: semtimedop");

            if(data.shm->initialized) {
                if(semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout))
                    return failure("init: semtimedop");
                break;
            }
            if(semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout))
                return failure("init: semtimedop");
            sleep(1);
        };
        return EXIT_SUCCESS;
    }
    const int colors[] = {RGB_RED_, RGB_GREEN_, RGB_BLUE_, RGB_CYAN_, RGB_YELLOW_, RGB_MAGENTA_};

    if(semtimedop(data.semid, &data.sem->init_lock, 1, &data.sem->timeout))
        return failure("init: semtimedop");

    data.shm->width = data.opt.width;
    data.shm->height = data.opt.height;
    data.shm->max_teams = data.opt.max_teams;
    data.shm->max_players = data.opt.max_players;

    for(ubyte x = 0; x < MAX_TEAMS; x++) {

        memset(data.shm->teams[x].name, 0, NAME_SIZE);
        data.shm->teams[x].color = colors[x];
    }
    const size_t player_size = sizeof(t_player);

    for(ushort x = 0; x < MAX_WIDTH; x++)
        for(ushort y = 0; y < MAX_HEIGHT; y++)
            memset(&data.shm->board[x][y], 0, player_size);

    data.shm->teams_count = 0;
    data.shm->players_count = 0;
    data.shm->initialized = YES;
    data.shm->gui = NO;

    if(semtimedop(data.semid, &data.sem->init_unlock, 1, &data.sem->timeout))
        return failure("init: semtimedop");

    return EXIT_SUCCESS;
}
