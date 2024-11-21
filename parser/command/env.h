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

#ifndef PARSER_COMMAND_ENV_H_
#define PARSER_COMMAND_ENV_H_

#if defined(__ANDROID__)
#define CURRENT_DIR_DEF "/data/local/tmp"
#else
#define CURRENT_DIR_DEF "."
#endif

class Env {
public:
    Env() : pid(0), remote_pid(0) {}
    void init();
    bool setCurrentPid(int p);
    inline int current() { return pid; }
    inline int remote() { return remote_pid; }

    static void Init();
    static void Dump();
    static void Clean();
    static bool SetCurrentPid(int p) { return INSTANCE->setCurrentPid(p); }
    static int CurrentPid() { return INSTANCE->current(); }
    static int CurrentRemotePid() { return INSTANCE->remote(); }
    static const char* CurrentDir() { return CURRENT_DIR_DEF; }
private:
    static Env* INSTANCE;
    int pid;
    int remote_pid;
};

#endif  // PARSER_COMMAND_ENV_H_
