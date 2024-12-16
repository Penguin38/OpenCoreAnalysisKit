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
#include "command/command_manager.h"
#include "command/help.h"
#include "command/cmd_time.h"
#include "command/cmd_core.h"
#include "command/cmd_exec.h"
#include "command/cmd_sysroot.h"
#include "command/cmd_mmap.h"
#include "command/cmd_auxv.h"
#include "command/cmd_file.h"
#include "command/cmd_linkmap.h"
#include "command/cmd_read.h"
#include "command/cmd_write.h"
#include "command/cmd_register.h"
#include "command/cmd_disassemble.h"
#include "command/cmd_thread.h"
#include "command/cmd_getprop.h"
#include "command/cmd_print.h"
#include "command/cmd_reference.h"
#include "command/cmd_hprof.h"
#include "command/cmd_search.h"
#include "command/cmd_class.h"
#include "command/cmd_top.h"
#include "command/cmd_space.h"
#include "command/cmd_dex.h"
#include "command/cmd_method.h"
#include "command/cmd_logcat.h"
#include "command/cmd_dumpsys.h"
#include "command/cmd_env.h"
#include "command/cmd_cxx.h"
#include "command/cmd_fdtrack.h"
#include "command/cmd_scudo.h"
#include "command/cmd_shell.h"
#include "command/remote/cmd_remote.h"
#include "command/fake/cmd_fake.h"
#include "command/backtrace/cmd_backtrace.h"
#include "command/backtrace/cmd_frame.h"
#include "common/exception.h"
#include "base/utils.h"
#include <string.h>

std::unique_ptr<CommandManager> CommandManager::INSTANCE;

class VersionCommand : public Command {
public:
    VersionCommand() : Command("version") {}
    ~VersionCommand() {}
    void usage() {}
    int main(int /*argc*/, char* const * /*argv[]*/) {
        LOGI("%s\n", Get().c_str());
        return 0;
    }
    inline static std::string Get() {
        std::string version;
        version.append("(");
        version.append(__TARGET_PARSER__);
        version.append(")");
        version.append(" ");
        version.append(__CORE_PARSER__);
        version.append(" ");
        version.append(__PARSER_VERSION__);
        return version;
    }
};

void CommandManager::Init() {
    INSTANCE = std::make_unique<CommandManager>();
    // core
    CommandManager::PushInlineCommand(new CoreCommand());
    CommandManager::PushInlineCommand(new ExecCommand());
    CommandManager::PushInlineCommand(new SysRootCommand());
    CommandManager::PushInlineCommand(new MmapCommand());
    CommandManager::PushInlineCommand(new AuxvCommand());
    CommandManager::PushInlineCommand(new FileCommand());
    CommandManager::PushInlineCommand(new LinkMapCommand());
    CommandManager::PushInlineCommand(new ReadCommand());
    CommandManager::PushInlineCommand(new WriteCommand());
    CommandManager::PushInlineCommand(new RegisterCommand());
    CommandManager::PushInlineCommand(new ThreadCommand());
    CommandManager::PushInlineCommand(new BacktraceCommand());
    CommandManager::PushInlineCommand(new FrameCommand());
    CommandManager::PushInlineCommand(new DisassembleCommand());

#if defined(__AOSP_PARSER__)
    // android
    CommandManager::PushInlineCommand(new GetPropCommand());
    CommandManager::PushInlineCommand(new PrintCommand());
    CommandManager::PushInlineCommand(new ReferenceCommand());
    CommandManager::PushInlineCommand(new HprofCommand());
    CommandManager::PushInlineCommand(new SearchCommand());
    CommandManager::PushInlineCommand(new ClassCommand());
    CommandManager::PushInlineCommand(new TopCommand());
    CommandManager::PushInlineCommand(new SpaceCommand());
    CommandManager::PushInlineCommand(new DexCommand());
    CommandManager::PushInlineCommand(new MethodCommand());
    CommandManager::PushInlineCommand(new LogcatCommand());
    CommandManager::PushInlineCommand(new DumpsysCommand());
    CommandManager::PushInlineCommand(new FdtrackCommand());
#endif

    // other
    CommandManager::PushInlineCommand(new EnvCommand());
    CommandManager::PushInlineCommand(new CxxCommand());
    CommandManager::PushInlineCommand(new ScudoCommand());
    CommandManager::PushInlineCommand(new ShellCommand());
    INSTANCE->plugin = new PluginCommand();
    CommandManager::PushInlineCommand(INSTANCE->plugin);
    CommandManager::PushInlineCommand(new Help());
#if !defined(__MACOS__)
    CommandManager::PushInlineCommand(new RemoteCommand());
#endif
    CommandManager::PushInlineCommand(new FakeCommand());
    CommandManager::PushInlineCommand(new TimeCommand());
    CommandManager::PushInlineCommand(new VersionCommand());
}

int CommandManager::Execute(const char* cmd, int argc, char* const argv[]) {
    if (!cmd) return -1;
    Command* command = FindCommand(cmd);
    if (command) {
        try {
            ShellCommand* shell = nullptr;
            int position = 0;
            bool writeout = false;
            for (int i = 0; i < argc; ++i) {
                if (!strcmp(argv[i], "|")) {
                    ++i;
                    if (i < argc) {
                        shell = reinterpret_cast<ShellCommand *>(FindCommand("shell"));
                        position = i;
                        break;
                    }
                } else if (!strcmp(argv[i], ">")) {
                    ++i;
                    if (i < argc) {
                        position = i;
                        writeout = true;
                        break;
                    }
                }
            }

            if (shell) {
                int nargc = position - 1;
                auto callback = [&command, nargc, &argv]() {
                    command->execute(nargc, argv);
                };
                shell->main(argc - nargc, &argv[nargc], callback);
                return 0;
            } else if (writeout) {
                int fd = Utils::FreopenWrite(argv[position]);
                int nargc = position - 1;
                command->execute(nargc, argv);
                Utils::CloseWriteout(fd);
                return 0;
            }
            return command->execute(argc, argv);
        } catch (InvalidAddressException e) {
            LOGE("%s\n", e.what());
        }
    } else {
        LOGW("Not found command (%s).\n", cmd);
    }
    return 0;
}

void CommandManager::ForeachCommand(std::function<bool (Command *)> callback) {
    INSTANCE->foreachInlineCommand(callback);
    INSTANCE->foreachExtendCommand(callback);
}

Command* CommandManager::FindCommand(const char* cmd) {
    Command* result = nullptr;
    auto callback = [cmd, &result](Command* command) -> bool {
        if (command->get() == cmd || command->shortcut() == cmd) {
            result = command;
            return true;
        }
        return false;
    };
    ForeachCommand(callback);
    return result;
}

void CommandManager::foreachInlineCommand(std::function<bool (Command *)> callback) {
    for (const auto& command : inline_commands)
        if (callback(command.get()))
            break;
}

void CommandManager::foreachExtendCommand(std::function<bool (Command *)> callback) {
    for (const auto& command : extend_commands)
        if (callback(command.get()))
            break;
}

void CommandManager::PushInlineCommand(Command* command) {
    INSTANCE->pushInlineCommand(command);
}

void CommandManager::pushInlineCommand(Command* command) {
    std::unique_ptr<Command> ref(command);
    inline_commands.push_back(std::move(ref));
}

int CommandManager::PushExtendCommand(Command* command) {
    return INSTANCE->pushExtendCommand(command);
}

int CommandManager::pushExtendCommand(Command* command) {
    if (!command)
        return 0;

    std::unique_ptr<Command> ref(command);
    Command* tmp = FindCommand(command->get().c_str());
    if (tmp) {
        LOGW("plugin \"%s\" exist.\n", command->get().c_str());
        return 0;
    }

    extend_commands.push_back(std::move(ref));
    plugin->GetPlugins()[plugin->GetPlugins().size() - 1]->HookCommand(command);
    return 1;
}

int CommandManager::PopExtendCommand(Command* command) {
    return INSTANCE->popExtendCommand(command);
}

int CommandManager::popExtendCommand(Command* command) {
    std::vector<std::unique_ptr<Command>>::iterator iter;
    for (iter = extend_commands.begin(); iter != extend_commands.end(); iter++) {
        if (iter->get() == command) {
            extend_commands.erase(iter);
            return 1;
        }
    }
    return 0;
}
