/*
 * Copyright (C) 2025-present, Guanyou.Chen. All rights reserved.
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
#include "base/utils.h"
#include "daemon/server.h"
#include "work/work_thread.h"
#include "command/env.h"
#include "sys/time.h"
#include <unistd.h>
#include <errno.h>
#include <string>
#include <fstream>

#ifdef __HTTPLIB__
#include "httplib.h"
#endif // __HTTPLIB__

bool CoreServer::start(const char* host) {
#ifdef __HTTPLIB__
    if (!host) {
        LOGE("Enter --daemon ip:port\n");
        return false;
    }

    std::string str = host;
    std::regex pattern(R"(^(\d{1,3})\.(\d{1,3})\.(\d{1,3})\.(\d{1,3}):(\d{1,5})$)");
    std::smatch match;
    if (!std::regex_match(str, match, pattern)) {
        LOGE("parse fail ip:port\n");
        return false;
    }

    for (int i = 1; i <= 4; ++i) {
        int num = std::stoi(match[i]);
        if (num < 0 || num > 255) {
            LOGE("ip out range.");
            return false;
        }
    }

    int port = std::stoi(match[5]);
    if (port < 0 || port > 65535) {
        LOGE("port out range.");
        return false;
    }

    size_t colon = str.find(':');
    std::string ip = str.substr(0, colon);

    Logger::SetHighLight(false);
    httplib::Server svr;

    std::string licenses;
    licenses += "<p>Copyright (C) 2024-present, Guanyou.Chen. All rights reserved.<br>";
    licenses += "<br>";
    licenses += "Licensed under the Apache License, Version 2.0 (the \"License\");<br>";
    licenses += "you may not use this file except in compliance with the License.<br>";
    licenses += "You may obtain a copy of the License at<br>";
    licenses += "<br>";
    licenses += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;http://www.apache.org/licenses/LICENSE-2.0<br>";
    licenses += "<br>";
    licenses += "Unless required by applicable law or agreed to in writing, software<br>";
    licenses += "distributed under the License is distributed on an \"AS IS\" BASIS,<br>";
    licenses += "WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.<br>";
    licenses += "See the License for the specific language governing permissions and<br>";
    licenses += "limitations under the License.<br>";
    licenses += "<br>";
    licenses += "For bug reporting instructions, please see:<br>";
    licenses += "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;https://github.com/Penguin38/OpenCoreAnalysisKit<br></p>";

    svr.Get("/", [&](const httplib::Request& req, httplib::Response& res) {
        res.set_content(licenses.c_str(), "text/html");
    });

    svr.Get("/core-parser", [&](const httplib::Request& req, httplib::Response& res) {
        std::string cmd = req.get_param_value("cmd");
        if (cmd.size() >= 2 && cmd.front() == '"' && cmd.back() == '"') {
            cmd = cmd.substr(1, cmd.size() - 2);
        }

        struct timeval tv;
        gettimeofday(&tv, NULL);
        std::string json;
        json.append(Env::CurrentDir());
        json.append("/core-parser-resp.");
        json.append(std::to_string(tv.tv_sec));
        json.append(".json");
        int fd = Utils::FreopenWrite(json.c_str());
        WorkThread work(cmd);
        work.Join();
        Utils::CloseWriteout(fd);

        std::ifstream file(json);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        file.close();
        unlink(json.c_str());

        // std::string content;
        // content.append("{\"result\": \"");
        // content.append(output);
        // content.append("\"}");
        res.set_content(content.c_str(), "application/json");
    });

    LOGI("Server running on http://%s:%d\n", ip.c_str(), port);
    if (daemon(1, 1) == -1) {
        LOGE("Failed to create daemon: %s\n", strerror(errno));
        return false;
    }
    running = true;
    svr.listen(ip, port);
    return true;
#else
    LOGE("Doesn't support 32-bit platforms.\n");
    return false;
#endif
}
