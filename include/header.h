#ifndef HEADER_H
#define HEADER_H
#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <mlx.h>
#include "define.h"
#include "struct.h"

void getargs(const int ac, char** const av);
byte init();
byte join();
byte draw();
byte bye();

#endif
