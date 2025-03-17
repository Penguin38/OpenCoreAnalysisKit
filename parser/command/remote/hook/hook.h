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

#ifndef PARSER_COMMAND_REMOTE_HOOK_HOOK_H_
#define PARSER_COMMAND_REMOTE_HOOK_HOOK_H_

#include <memory>

class Hook {
public:
    Hook(int p) : pid(p) {}
    virtual ~Hook() {}
    virtual uint64_t RegsSize() { return 0x0; }
    virtual bool InjectLibrary(const char* library) { return false; }
    virtual bool CallMethod(const char* method, int argc, char* const argv[]) { return false; }

    inline int Pid() { return pid; }
    bool Continue();
    bool LoadContext(void *regs);
    bool StoreContext(void *regs);

    struct MethodTable {
        const char* method;
        const char* sign;
        int min;
    };

    static int Main(int argc, char* const argv[]);
    static void Usage();
    static std::unique_ptr<Hook> MakeArch(int pid);
private:
    int pid;
};

#endif // PARSER_COMMAND_REMOTE_HOOK_HOOK_H_
