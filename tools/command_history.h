#ifndef HISTORY_H
#define HISTORY_H

#define HISTORY_SIZE 4
#define CMD_LEN 128

#define KEY_UP   0x100
#define KEY_DOWN 0x101

void history_add(const char *cmd);

const char* history_get_prev(void);

const char* history_get_next(void);

#endif