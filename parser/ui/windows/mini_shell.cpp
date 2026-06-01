/*
 * Copyright (C) 2026-present, Guanyou.Chen. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "logger/log.h"
#include "ui/mini_shell.h"
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <conio.h>
#undef THIS

static DWORD sOrigConsoleMode;

void MiniShell::Init(const char* sh) {
    if (sh) shell = sh;
    shell.append("> ");
    minX = shell.length();
    memset(history, 0x0, HISTORY_SIZE * MAX_CMD_LENGTH);
}

void MiniShell::EnableMode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hStdin, &sOrigConsoleMode);
    SetConsoleMode(hStdin, sOrigConsoleMode & ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT));
}

void MiniShell::DisableMode() {
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(hStdin, sOrigConsoleMode);
}

void MiniShell::SaveHistory(char* cmd) {
    if (cmd[0] == '\0') return;

    if (history_counter < HISTORY_SIZE) {
        strcpy(history[history_counter], cmd);
        history_counter++;
        history_cursor = history_counter;
    } else {
        for (int i = 1; i < HISTORY_SIZE; ++i) {
            strcpy(history[i - 1], history[i]);
        }
        strcpy(history[history_counter - 1], cmd);
        history_cursor = history_counter;
    }
}

void MiniShell::ShowHeader() {
    printf(ANSI_COLOR_RESET "%s", shell.c_str());
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
        int ch = _getch();
        if (ch == EOF) break;

        // Windows _getch() returns 0xE0 or 0x00 prefix for special keys
        if (ch == 0xE0 || ch == 0x00) {
            int key = _getch();
            switch (key) {
                case 0x48: ShowHistory(true); break;   // Up
                case 0x50: ShowHistory(false); break;  // Down
                case 0x4D: CursorMove(1, 0); break;   // Right
                case 0x4B: CursorMove(-1, 0); break;  // Left
                case 0x47:                             // Home
                    if (curX - minX > 1)
                        CursorMove(minX + 1 - curX, 0);
                    break;
                case 0x4F:                             // End
                    if (curX < totalX)
                        CursorMove(totalX - curX, 0);
                    break;
            }
            continue;
        }

        seq[0] = (char)ch;

        // Also handle ESC sequences (e.g. Windows Terminal sends VT sequences)
        if (seq[0] == KEYCODE_ESC) {
            seq[1] = (char)_getch();
            if (seq[1] != 0x5B) break;
            seq[2] = (char)_getch();
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
