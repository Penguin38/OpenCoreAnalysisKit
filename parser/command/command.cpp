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

#include "command/command.h"
#include "common/exception.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

int Command::execute(int argc, char* const argv[]) {
    int state = prepare(argc, argv);
    if (state == ONCHLD) {
        pid_t pid = fork();
        int status;
        if (pid == 0) {
            struct sigaction stact;
            memset(&stact, 0, sizeof(stact));
            stact.sa_handler = Command::Exit;
            sigaction(SIGINT, &stact, NULL);
            sigaction(SIGTERM, &stact, NULL);
            try {
                main(argc, argv);
            } catch (InvalidAddressException& e) {
                LOGE("%s\n", e.what());
            }
            exit(0);
        } else if (pid > 0) {
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != ERREXIT) {
                exitMain(argc, argv);
            }
        }
        return 0;
    } else if (state == CONTINUE) {
        return main(argc, argv);
    }
    return FINISH;
}
