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

#include "daemon/mcp/server.h"
#include "logger/log.h"
#include "base/utils.h"
#include "work/work_thread.h"
#include <string>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <sys/time.h>

#ifndef __PARSER_VERSION__
#define __PARSER_VERSION__ "unknown"
#endif

// ── Minimal JSON helpers ──────────────────────────────────────────────────
//
// We avoid pulling in a full JSON library.  The MCP wire format is
// well-structured enough that simple string scanning is sufficient.

namespace {

// Return the value of the first occurrence of "key":"value" in json.
// Handles escaped characters inside the value string.
static std::string JsonGetString(const std::string& json, const std::string& key) {
    std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "";
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) return "";
    pos = json.find('"', pos + 1);
    if (pos == std::string::npos) return "";
    size_t end = pos + 1;
    while (end < json.size()) {
        if (json[end] == '\\') { end += 2; continue; }
        if (json[end] == '"')  break;
        end++;
    }
    if (end >= json.size()) return "";
    return json.substr(pos + 1, end - pos - 1);
}

// Return the raw JSON representation of "id" (number, quoted string, or "null").
// The returned string is safe to embed directly in a JSON response.
static std::string JsonGetId(const std::string& json) {
    std::string needle = "\"id\"";
    size_t pos = json.find(needle);
    if (pos == std::string::npos) return "null";
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) return "null";
    pos++;
    while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t')) pos++;
    if (pos >= json.size()) return "null";
    if (json[pos] == '"') {
        // string id — return including the surrounding quotes
        size_t end = pos + 1;
        while (end < json.size()) {
            if (json[end] == '\\') { end += 2; continue; }
            if (json[end] == '"')  { end++; break; }
            end++;
        }
        return json.substr(pos, end - pos);
    }
    // numeric id
    size_t end = pos;
    while (end < json.size() && (std::isdigit((unsigned char)json[end]) || json[end] == '-'))
        end++;
    return end > pos ? json.substr(pos, end - pos) : "null";
}

// Reverse JSON string escaping (\\n → \n, \\" → ", etc.)
static std::string JsonUnescape(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] != '\\' || i + 1 >= s.size()) { out += s[i]; continue; }
        switch (s[++i]) {
            case '"':  out += '"';  break;
            case '\\': out += '\\'; break;
            case '/':  out += '/';  break;
            case 'n':  out += '\n'; break;
            case 'r':  out += '\r'; break;
            case 't':  out += '\t'; break;
            default:   out += '\\'; out += s[i]; break;
        }
    }
    return out;
}

// Escape an arbitrary string for embedding as a JSON string value.
static std::string JsonEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 16);
    for (unsigned char c : s) {
        switch (c) {
            case '"':  out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n";  break;
            case '\r': out += "\\r";  break;
            case '\t': out += "\\t";  break;
            default:
                if (c < 0x20) {
                    char buf[8];
                    snprintf(buf, sizeof(buf), "\\u%04x", c);
                    out += buf;
                } else {
                    out += c;
                }
        }
    }
    return out;
}

// ── Low-level I/O ─────────────────────────────────────────────────────────

// Write a complete message to fd, bypassing the stdout FILE* buffer.
// This avoids any state confusion that could arise from dup2-ing fd 1
// back and forth during command capture.
static void WriteAll(int fd, const std::string& msg) {
    const char* p = msg.c_str();
    size_t rem = msg.size();
    while (rem > 0) {
        ssize_t n = write(fd, p, rem);
        if (n <= 0) break;
        p += n; rem -= n;
    }
}

// Emit a JSON-RPC 2.0 success response, terminated by a newline.
static void SendResponse(int fd, const std::string& id, const std::string& result_json) {
    std::string msg;
    msg.reserve(64 + result_json.size());
    msg += "{\"jsonrpc\":\"2.0\",\"id\":";
    msg += id;
    msg += ",\"result\":";
    msg += result_json;
    msg += "}\n";
    WriteAll(fd, msg);
}

// Emit a JSON-RPC 2.0 error response.
static void SendError(int fd, const std::string& id, int code, const std::string& message) {
    std::string msg;
    msg += "{\"jsonrpc\":\"2.0\",\"id\":";
    msg += id;
    msg += ",\"error\":{\"code\":";
    msg += std::to_string(code);
    msg += ",\"message\":\"";
    msg += JsonEscape(message);
    msg += "\"}}\n";
    WriteAll(fd, msg);
}

// ── Command execution with output capture ─────────────────────────────────

// Run a core-parser command and return its stdout output as a string.
// Uses the same dup2-based capture mechanism as the HTTP daemon.
static std::string RunCommand(const std::string& cmd) {
    struct timeval tv;
    gettimeofday(&tv, nullptr);

    std::string tmppath = "/tmp/core-parser-mcp.";
    tmppath += std::to_string(static_cast<long>(tv.tv_sec));
    tmppath += ".";
    tmppath += std::to_string(static_cast<long>(tv.tv_usec));
    tmppath += ".txt";

    int saved_fd = Utils::FreopenWrite(tmppath.c_str());
    WorkThread work(cmd);
    work.Join();
    Utils::CloseWriteout(saved_fd);

    std::ifstream f(tmppath);
    std::string content((std::istreambuf_iterator<char>(f)),
                         std::istreambuf_iterator<char>());
    f.close();
    unlink(tmppath.c_str());
    return content;
}

// ── MCP request handlers ──────────────────────────────────────────────────

static void HandleInitialize(int fd, const std::string& id) {
    std::string result =
        "{\"protocolVersion\":\"2024-11-05\","
        "\"capabilities\":{\"tools\":{}},\"serverInfo\":{\"name\":\"core-parser\","
        "\"version\":\"" __PARSER_VERSION__ "\"}}";
    SendResponse(fd, id, result);
}

static void HandleToolsList(int fd, const std::string& id) {
    // Single tool: run_command
    // The description gives Claude enough context to use the tool effectively.
    static const char* kResult =
        "{\"tools\":[{"
            "\"name\":\"run_command\","
            "\"description\":"
                "\"Execute a core-parser command on the loaded core/tombstone and return "
                "the text output.\\n\\n"
                "Commonly used commands:\\n"
                "  core             loaded corefile\\n"
                "  bt               backtrace all threads\\n"
                "  thread           list threads with status\\n"
                "  f <N>            frame N: registers + disassembly\\n"
                "  register         dump registers of crash thread\\n"
                "  map              full VMA / memory map\\n"
                "  print <addr>     dump Java object at address\\n"
                "  method <addr>    dump ArtMethod dexdump or oatdump\\n"
                "  disassemble      disassemble instructions around PC\\n"
                "  ini              show/edit ART struct offset tables\\n"
                "  help             list all available commands\","
            "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                    "\"command\":{"
                        "\"type\":\"string\","
                        "\"description\":\"The core-parser command to run, "
                            "e.g. \\\"bt\\\", \\\"f 0\\\", \\\"print 0x12345678\\\", \\\"mmap\\\"\""
                    "}"
                "},"
                "\"required\":[\"command\"]"
            "}"
        "}]}";
    SendResponse(fd, id, kResult);
}

static void HandleToolsCall(int fd, const std::string& id, const std::string& req) {
    std::string tool_name = JsonGetString(req, "name");
    if (tool_name != "run_command") {
        SendError(fd, id, -32602, "Unknown tool: " + tool_name);
        return;
    }

    // Params layout: {"name":"run_command","arguments":{"command":"..."}}
    // Narrow the search to the "arguments" sub-object to avoid collisions
    // with the outer "name" field.
    std::string cmd;
    size_t args_pos = req.find("\"arguments\"");
    if (args_pos != std::string::npos) {
        cmd = JsonUnescape(JsonGetString(req.substr(args_pos), "command"));
    }

    if (cmd.empty()) {
        SendError(fd, id, -32602, "Missing required argument: command");
        return;
    }

    std::string output = RunCommand(cmd);

    std::string result =
        "{\"content\":[{\"type\":\"text\",\"text\":\""
        + JsonEscape(output)
        + "\"}]}";
    SendResponse(fd, id, result);
}

} // namespace

// ── Public entry point ────────────────────────────────────────────────────

void McpServer::start() {
    // Disable ANSI color codes — output is captured to temp files and
    // then embedded as plain text inside JSON.
    Logger::SetHighLight(false);
    // Raise the log level so warnings are visible in captured output.
    // Logger::SetLevel(Logger::LEVEL_WARN);

    // Save the real stdout fd before any dup2 manipulation.
    // All JSON-RPC responses are written to this fd directly, bypassing
    // the FILE* layer to avoid buffering surprises.
    int stdout_fd = dup(fileno(stdout));

    std::string line;
    while (std::getline(std::cin, line)) {
        // Strip trailing CR from CRLF line endings.
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;

        std::string method = JsonGetString(line, "method");
        std::string id     = JsonGetId(line);

        if (method == "initialize") {
            HandleInitialize(stdout_fd, id);
        } else if (method == "notifications/initialized") {
            // Notification: no response required.
        } else if (method == "ping") {
            SendResponse(stdout_fd, id, "{}");
        } else if (method == "tools/list") {
            HandleToolsList(stdout_fd, id);
        } else if (method == "tools/call") {
            HandleToolsCall(stdout_fd, id, line);
        } else if (!method.empty()) {
            SendError(stdout_fd, id, -32601, "Method not found: " + method);
        }
    }

    close(stdout_fd);
}
