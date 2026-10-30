#include "command_history.h"

static char history[HISTORY_SIZE][CMD_LEN];
static int history_count = 0;
static int history_pos = 0;

static void my_strcpy(char *dest, const char *src) {
    while ((*dest++ = *src++));
}

void history_add(const char *cmd) {
    if (cmd[0] == '\0') return;

    if (history_count < HISTORY_SIZE) {
        my_strcpy(history[history_count], cmd);
        history_count++;
    } else {
        for (int i = 0; i < HISTORY_SIZE - 1; i++) {
            my_strcpy(history[i], history[i + 1]);
        }
        my_strcpy(history[HISTORY_SIZE - 1], cmd);
    }
    history_pos = history_count;
}

const char* history_get_prev(void) {
    if (history_count > 0 && history_pos > 0) {
        history_pos--;
        return history[history_pos];
    }
    return 0;
}

const char* history_get_next(void) {
    if (history_pos < history_count) {
        history_pos++;
        if (history_pos < history_count) {
            return history[history_pos];
        } else {
            return "";
        }
    }
    return 0;
}