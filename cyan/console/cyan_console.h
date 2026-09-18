#ifndef CYAN_CONSOLE_H
#define CYAN_CONSOLE_H

#ifdef __cplusplus
extern "C" {
#endif

void cyan_console_init(void);
void cyan_console_poll(void);
void cyan_console_shutdown(void);

// Prints `prompt` followed by " (y/n)" and puts the console into confirmation mode: the next
// line the user submits is interpreted as yes/no rather than dispatched as a shell command. On a
// "y"/"yes" answer (case-insensitive) `onConfirm` is called; anything else cancels silently.
// Only one confirmation can be pending at a time - a new request replaces any prior one.
void cyan_console_request_confirmation(const char* prompt, void (*onConfirm)(void));

#ifdef __cplusplus
}
#endif

#endif // CYAN_CONSOLE_H
