//platform/pc/main_pc.c
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "../../cyan/cyan_os.h"

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;

    if (!cyan_init()) {
        printf("Failed to initialise Cyberwatch\n");
        return 1;
    }


    clock_t lastTime = clock();
    bool running = true;
    while (running) {
        clock_t now = clock();
        float dt = (float) (now - lastTime) / CLOCKS_PER_SEC;
        lastTime = now;
        cyan_update(dt, &running);
    }

    cyan_shutdown();
    return 0;
}