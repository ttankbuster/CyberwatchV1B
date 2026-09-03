// cyan_os.h
#ifndef CYAN_OS_H
#define CYAN_OS_H

#include "app_handling/app_handler.h"
#include "data/data.h"
#include "data/display.h"
#include <stdbool.h>

#define CYAN_VERSION "Cyan V1B"

extern CyanData data;
extern Display display;

bool cyan_init(void);
void cyan_update(float dt, bool* running);
void cyan_shutdown(void);

bool cyan_launch_app_id(int id);
bool cyan_launch_app_name(char* name);
bool cyan_is_app_running();
bool cyan_exit_app();
#endif