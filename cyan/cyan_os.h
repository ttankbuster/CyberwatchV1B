//cyan_os.h
#ifndef CYBERWATCH_H
#define CYBERWATCH_H

#include <stdbool.h>
#include "data/data.h"
#include "data/display.h"
#include "app_handling/app_handler.h"

extern CyberwatchData data;
extern Display display;

bool cyan_init(void);
void cyan_update(float dt, bool *running);
void cyan_shutdown(void);

#endif