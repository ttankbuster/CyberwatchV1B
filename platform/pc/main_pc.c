//platform/pc/main_pc.c
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include "../../src/cyberwatch.h"

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    printf("main_pc.c\n");

    if (!cyberwatch_init()) {
        printf("Failed to initialise Cyberwatch\n");
        return 1;
    }
    printf("setup complete: starting.\n");

    clock_t lastTime = clock();
    bool running = true;
    while (running) {
        clock_t now = clock();
        float dt = (float) (now - lastTime) / CLOCKS_PER_SEC;
        lastTime = now;
        cyberwatch_update(dt, &running);
    }

    cyberwatch_shutdown();
    return 0;
}