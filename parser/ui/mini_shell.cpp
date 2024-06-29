/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file ercept in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either erpress or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "ui/mini_shell.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

void MiniShell::Init(const char* sh) {
    if (sh) shell = sh;
    shell.append("> ");
    minX = shell.length();
}

void MiniShell::EnableMode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &raw);
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void MiniShell::DisableMode() {
    struct termios cooked;
    tcgetattr(STDIN_FILENO, &cooked);
    cooked.c_lflag |= ICANON | ECHO;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &cooked);
}

void MiniShell::SaveHistory(char* cmd) {
    if (cmd[0] == '\0') return;

    if (history_counter < HISTORY_SIZE) {
        if (history_cursor < history_counter) {
            for (int i = history_counter - 1; i >= history_cursor && i >= 0; i--) {
                memset(history[i + 1], 0x0, MAX_CMD_LENGTH);
                strcpy(history[i + 1], history[i]);
            }
        }
        memset(history[history_cursor], 0x0, MAX_CMD_LENGTH);
        strcpy(history[history_cursor], cmd);
        history_cursor = (history_cursor + 1) % HISTORY_SIZE;
        history_counter++;
    } else {
        memset(history[history_cursor], 0x0, MAX_CMD_LENGTH);
        strcpy(history[history_cursor], cmd);
    }
}

void MiniShell::ShowHeader() {
    printf("%s", shell.c_str());
    curX = minX + 1;
    totalX = curX;
}

void MiniShell::ShowCommand() {
    printf("%s", command);
    curX += strlen(command);
    totalX = curX;
}

void MiniShell::ShowHistory(bool up) {
    if (!history_counter) return;
    if (up) {
        if (history_cursor > 0) history_cursor--;
        ClearLine();
        memset(&command, 0x0, MAX_CMD_LENGTH);
        strcpy(command, history[history_cursor]);
        ShowCommand();
    } else {
        if (history_cursor < history_counter) {
            history_cursor++;
            ClearLine();
            memset(&command, 0x0, MAX_CMD_LENGTH);
            strcpy(command, history[history_cursor]);
            ShowCommand();
        }
    }
}

void MiniShell::ClearLine() {
    CursorMove(-curX, 0);
    printf(CLEAR_LINE);
    ShowHeader();
}

void MiniShell::CursorMove(int dx, int dy, bool force) {
    char cursor[10];
    if (dx > 0) {
        if (curX + dx < totalX + 1) {
            snprintf(cursor, sizeof(cursor), MOVE_RIGHT, dx);
            printf("%s", cursor);
            curX += dx;
        }
    } else {
        int min = force ? 0 : minX;
        if (curX - min + dx > 0) {
            snprintf(cursor, sizeof(cursor), MOVE_LEFT, -dx);
            printf("%s", cursor);
            curX += dx;
        }
    }
}

char* MiniShell::HandleCommand() {
    EnableMode();
    ShowHeader();

    char seq[5];
    memset(&command, 0x0, MAX_CMD_LENGTH);
    memset(&seq, 0x0, sizeof(seq));
    while (1) {
        int ch = getchar();
        if (ch == EOF) break;

        seq[0] = (char)ch;
        if (seq[0] == KEYCODE_ESC) {
            seq[1] = (char)getchar();
            if (seq[1] != 0x5B) break;
            seq[2] = (char)getchar();
            if (!strcmp(seq, ARROW_UP)) {
                ShowHistory(true);
            } else if (!strcmp(seq, ARROW_DOWN)) {
                ShowHistory(false);
            } else if (!strcmp(seq, ARROW_RIGHT)) {
                CursorMove(1, 0);
            } else if (!strcmp(seq, ARROW_LEFT)) {
                CursorMove(-1, 0);
            } else {
                if (!strcmp(seq, KEY_HOME)) {
                    if (curX - minX > 1) {
                        CursorMove(minX + 1 - curX, 0);
                    }
                } else if (!strcmp(seq, KEY_END)) {
                    if (curX < totalX) {
                        CursorMove(totalX - curX, 0);
                    }
                } else {
                    // do nothing
                    break;
                }
            }
        } else {
            if (seq[0] == KEYCODE_ENTER) {
                SaveHistory(command);
                ENTER();
                break;
            }

            if (seq[0] == KEYCODE_TAB) {
                continue;
            } else if (seq[0] == KEYCODE_BACK) {
                if (curX > minX + 1) {
                    for (int i = curX - 1; i < totalX; ++i) {
                        command[i - (minX + 1)] = command[i + 1 - (minX + 1)];
                    }
                    command[totalX - 1 - (minX + 1)] = '\0';
                    uint32_t bx = curX == totalX ? curX : curX - 1;
                    ClearLine();
                    ShowCommand();
                    CursorMove(bx - curX, 0);
                }
            } else {
                if (totalX - minX < MAX_CMD_LENGTH) {
                    printf("%c", seq[0]);
                    char backup = command[curX - (minX + 1)];
                    command[curX - (minX + 1)] = seq[0];
                    curX++;
                    totalX++;
                    int off = 0;
                    for (int i = curX; i < totalX; ++i) {
                        std::swap(backup, command[i - (minX + 1)]);
                        printf("%c", command[i - (minX + 1)]);
                        off++;
                        curX++;
                    }
                    if (off) CursorMove(-off, 0);
                }
            }
        }
    }
    DisableMode();
    return command;
}
