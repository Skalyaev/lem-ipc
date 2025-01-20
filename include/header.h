#ifndef HEADER_H
#define HEADER_H
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>
#include <time.h>

#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/time.h>

#include <mlx.h>
#include "define.h"
#include "struct.h"

void getargs(const int ac, char** const av);
byte init();
byte join();
byte bye();

byte player_check();
byte player_think();
byte player_communicate();
byte player_move();
byte player_log();

byte draw();
byte pause_game();
byte stop_game();
byte add_player(const ubyte team);

#endif
