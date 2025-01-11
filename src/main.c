#include "../include/header.h"

t_lemipc data = {0};

static byte bye() {

    return data.code;
}

int main(int ac, char** av) {

    return bye();
}
