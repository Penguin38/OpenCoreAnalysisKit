/*
 * Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.
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

#ifndef PARSER_UI_MINI_SHELL_H_
#define PARSER_UI_MINI_SHELL_H_

#include <stdint.h>
#include <sys/types.h>
#include <string>

#define MAX_CMD_LENGTH 512
#define HISTORY_SIZE 50

#define ARROW_UP     "\x1b[A"
#define ARROW_DOWN   "\x1b[B"
#define ARROW_RIGHT  "\x1b[C"
#define ARROW_LEFT   "\x1b[D"

#define MOVE_START   "\x1b[H"
#define CLEAR_LINE   "\x1b[K"

#define KEY_HOME     "\x1b[\x48"
#define KEY_END      "\x1b[\x46"

#define MOVE_UP      "\x1b[%dA"
#define MOVE_DOWN    "\x1b[%dB"
#define MOVE_RIGHT   "\x1b[%dC"
#define MOVE_LEFT    "\x1b[%dD"

#define KEYCODE_ESC    0x1B
#define KEYCODE_ENTER  0x0A
#define KEYCODE_BACK   0x7F
#define KEYCODE_TAB    0x09

class MiniShell {
public:
    void Init(const char* shell);
    void EnableMode();
    void DisableMode();
    void SaveHistory(char* cmd);
    void ShowHeader();
    void ShowHistory(bool up);
    void ShowCommand();
    void ClearLine();
    void CursorMove(int dx, int dy, bool force=false);
    char* HandleCommand();
private:
    /*
     *  ----- -----
     * |  i  | CMD |
     *  ----- -----
     * |  j  | CMD | --- cursor = j + 1;
     *  ----- -----  |   counter = cursor;
     * | --- | --- |<-
     *  ----- -----
     */
    uint32_t history_counter = 0;
    uint32_t history_cursor = 0;
    char history[HISTORY_SIZE][MAX_CMD_LENGTH];

    /*     Y
     *     |
     *(0,0)|shell> [cmd] [option] ..
     *     --------|-----------------X
     *            minX
     */
    uint32_t curX = 0;
    uint32_t curY = 0;
    uint32_t minX = 0;
    uint32_t minY = 0;
    uint32_t totalX = 0;
    char command[MAX_CMD_LENGTH] = "";

    std::string shell;
};

#endif  // PARSER_UI_MINI_SHELL_H_
