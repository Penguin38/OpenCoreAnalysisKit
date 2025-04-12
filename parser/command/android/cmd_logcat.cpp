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

#include "api/core.h"
#include "logger/log.h"
#include "android.h"
#include "common/exception.h"
#include "command/command_manager.h"
#include "command/android/cmd_logcat.h"
#include "logcat/log.h"
#include "logcat/LogBuffer.h"
#include "logcat/LogStatistics.h"
#include "logcat/SerializedData.h"
#include "logcat/SerializedLogBuffer.h"
#include "cxx/list.h"
#include <string>
#include <unistd.h>
#include <getopt.h>

using namespace android;

// std::list<SerializedLogChunk> logs_[LOG_ID_MAX] GUARDED_BY(logd_lock);
static void PrintSerializedLogBuf(const char* header, cxx::list& logs, int filter, int id) {
    LOGI("%s\n", header);
    try {
        for (const auto& value : logs) {
            SerializedData content = value;
            if (!content.data())
                continue;
            content.DecodeDump(filter, id);
        }
    } catch (InvalidAddressException& e) {
        LOGW("maybe loss of partial logs!!\n");
    }
}

int LogcatCommand::prepare(int argc, char* const argv[]) {
    if (!CoreApi::IsReady())
        return Command::FINISH;

    if (CoreApi::Bits() != 64) {
        LOGE("Not support logcat on bit32 system.\n");
        return Command::FINISH;
    }

    options.dump_flag = 0;
    options.filter = 0;
    options.id = 0;

    int opt;
    int option_index = 0;
    optind = 0; // reset
    static struct option long_options[] = {
        {"buffer",    required_argument,  0,  'b'},
        {"pid",       required_argument,  0,  'p'},
        {"uid",       required_argument,  0,  'u'},
        {"tid",       required_argument,  0,  't'},
        {0,           0,                  0,   0 },
    };

    while ((opt = getopt_long(argc, argv, "b:p:u:t:",
                long_options, &option_index)) != -1) {
        switch (opt) {
            case 'b':
                if (!strcmp(optarg, "main"))
                    options.dump_flag |= DUMP_MAIN;

                if (!strcmp(optarg, "radio"))
                    options.dump_flag |= DUMP_RADIO;

                if (!strcmp(optarg, "events"))
                    options.dump_flag |= DUMP_EVENTS;

                if (!strcmp(optarg, "system"))
                    options.dump_flag |= DUMP_SYSTEM;

                if (!strcmp(optarg, "crash"))
                    options.dump_flag |= DUMP_CRASH;

                if (!strcmp(optarg, "kernel"))
                    options.dump_flag |= DUMP_KERNEL;

                if (!strcmp(optarg, "all")) {
                    options.dump_flag = DUMP_MAIN | DUMP_RADIO
                                      | DUMP_EVENTS | DUMP_SYSTEM
                                      | DUMP_CRASH | DUMP_KERNEL;
                }
                break;
            case 'p':
                options.filter = SerializedData::FILTER_PID;
                options.id = std::atoi(optarg);
                break;
            case 'u':
                options.filter = SerializedData::FILTER_UID;
                options.id = std::atoi(optarg);
                break;
            case 't':
                options.filter = SerializedData::FILTER_TID;
                options.id = std::atoi(optarg);
                break;
        }
    }
    options.optind = optind;

    if (!options.dump_flag) {
        options.dump_flag = DUMP_MAIN
                          | DUMP_SYSTEM
                          | DUMP_CRASH
                          | DUMP_KERNEL;
    }

    return Command::ONCHLD;
}

int LogcatCommand::main(int argc, char* const argv[]) {
    if (Android::Sdk() >= Android::S) {
        SerializedLogBuffer log_buffer = Logcat::AnalysisSerializedLogBuffer();
        if (!log_buffer.Ptr()) {
            LOGE("Not found SerializedLogBuffer!\n");
            return 0;
        }

        if (options.dump_flag & DUMP_MAIN) {
            cxx::list main_logs = log_buffer.logs() + LOG_ID_MAIN * SIZEOF(cxx_list);
            PrintSerializedLogBuf("--------- beginning of main", main_logs, options.filter, options.id);
        }

        if (options.dump_flag & DUMP_RADIO) {
            cxx::list radio_logs = log_buffer.logs() + LOG_ID_RADIO * SIZEOF(cxx_list);
            PrintSerializedLogBuf("--------- beginning of radio", radio_logs, options.filter, options.id);
        }

        if (options.dump_flag & DUMP_EVENTS) {
            cxx::list events_logs = log_buffer.logs() + LOG_ID_EVENTS * SIZEOF(cxx_list);
            PrintSerializedLogBuf("--------- beginning of events", events_logs, options.filter, options.id);
        }

        if (options.dump_flag & DUMP_SYSTEM) {
            cxx::list system_logs = log_buffer.logs() + LOG_ID_SYSTEM * SIZEOF(cxx_list);
            PrintSerializedLogBuf("--------- beginning of system", system_logs, options.filter, options.id);
        }

        if (options.dump_flag & DUMP_CRASH) {
            cxx::list crash_logs = log_buffer.logs() + LOG_ID_CRASH * SIZEOF(cxx_list);
            PrintSerializedLogBuf("--------- beginning of crash", crash_logs, options.filter, options.id);
        }

        if (options.dump_flag & DUMP_KERNEL) {
            cxx::list kernel_logs = log_buffer.logs() + LOG_ID_KERNEL * SIZEOF(cxx_list);
            PrintSerializedLogBuf("--------- beginning of kernel", kernel_logs, options.filter, options.id);
        }
    } else {
    }
    return 0;
}

void LogcatCommand::usage() {
    LOGI("Usage: logcat [OPTION]...\n");
    LOGI("Option:\n");
    LOGI("    -b, --buffer <BUFFER>  collect only from buffers\n");
    LOGI("Buffer:{main, radio, events, system, crash, kernel}\n");
    LOGI("    -p, --pid <PID>        collect only from pid\n");
    LOGI("    -u, --uid <UID>        collect only from uid\n");
    LOGI("    -t, --tid <TID>        collect only from tid\n");
    ENTER();
    LOGI("core-parser> logcat -b crash -p 11770\n");
    LOGI("--------- beginning of crash\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: FATAL EXCEPTION: FinalizerWatchdogDaemon\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: Process: com.demo.app, PID: 11770\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: java.util.concurrent.TimeoutException: android.content.res.ApkAssets.finalize() timed out after 40 seconds\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Daemons$Daemon.isRunning(Unknown Source:0)\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Daemons$FinalizerDaemon.runInternal(Daemons.java:286)\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Daemons$Daemon.run(Daemons.java:140)\n");
    LOGI("2024-06-16 01:58:18.481  10232 11770 11784 E AndroidRuntime: 	at java.lang.Thread.run(Thread.java:1012)\n");
}
