#ifndef CYBERWATCH_H
#define CYBERWATCH_H

#include <stdbool.h>
#include "data.h"
#include "display.h"
#include "../cyan/cyan.h"

extern CyberwatchData data;
extern Display display;
extern Cyan cyan;

bool cyberwatch_init(void);
void cyberwatch_update(float dt, bool *running);
void cyberwatch_shutdown(void);

#endif