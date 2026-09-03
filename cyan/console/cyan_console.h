#ifndef CYAN_CONSOLE_H
#define CYAN_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

void cyan_console_init(void);
void cyan_console_poll(void);
void cyan_console_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif // CYAN_CONSOLE_H
