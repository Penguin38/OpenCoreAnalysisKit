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

#include "command/cmd_shell.h"
#include "base/utils.h"
#include "api/core.h"
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>

int ShellCommand::main(int argc, char* const argv[]) {
    return main(argc, argv, nullptr);
}

int ShellCommand::main(int argc, char* const argv[], std::function<void ()> callback) {
    if (!argc) 
        return 0;

	int fd[2];
	pipe(fd);

    int status;
    char *cmd = argv[0];

    pid_t pid = fork();
    if (pid == 0) {
        if (callback) {
            close(fd[1]);
            dup2(fd[0], 0);
            close(fd[0]);
        }
        if (execvp(cmd, argv) == -1)
            std::cout << "Unknown command " << cmd << ".  Try \"help\"." << std::endl;
        exit(0);
    } else if (pid > 0) {
        if (callback) {
            close(fd[0]);
            dup2(fd[1], 1);
            close(fd[1]);
            callback();
            dup2(fileno(stdin), 1);
        }
        waitpid(pid, &status, WUNTRACED);
    }

    return 0;
}

void ShellCommand::usage() {
    std::cout << "Usage: shell|sh cmd ..." << std::endl;
}
