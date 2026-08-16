//cyan_os.h
#ifndef CYAN_OS_H
#define CYAN_OS_H

#include <stdbool.h>
#include "data/data.h"
#include "data/display.h"
#include "app_handling/app_handler.h"

extern CyanData data;
extern Display display;

bool cyan_init(void);
void cyan_update(float dt, bool *running);
void cyan_shutdown(void);

#endif