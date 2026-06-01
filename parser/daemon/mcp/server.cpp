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
#ifdef __WINDOWS__
#include <winsock2.h>
#include <ws2tcpip.h>
#include <io.h>
#include <fcntl.h>
#undef THIS
#define sock_read(fd, buf, len)   recv(fd, (char*)(buf), len, 0)
#define sock_write(fd, buf, len)  send(fd, (const char*)(buf), len, 0)
#define sock_close(fd)            closesocket(fd)
#define dup(fd)                   _dup(fd)
#define fd_close(fd)              _close(fd)
#define write(fd, buf, len)       _write(fd, buf, (unsigned int)(len))
static int gettimeofday(struct timeval* tp, void*) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    t -= 116444736000000000ULL;
    tp->tv_sec = (long)(t / 10000000ULL);
    tp->tv_usec = (long)((t % 10000000ULL) / 10);
    return 0;
}
#else
#include <unistd.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define sock_read(fd, buf, len)   read(fd, buf, len)
#define sock_write(fd, buf, len)  write(fd, buf, len)
#define sock_close(fd)            close(fd)
#define fd_close(fd)              close(fd)
#endif

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

// ── Remote HTTP client (for run_remote_command) ───────────────────────────

// Cached base URL from the last connect_remote / run_remote_command call.
static std::string s_remote_url;

// Percent-encode a string for use as a URL query parameter value.
static std::string UrlEncode(const std::string& s) {
    static const char* kHex = "0123456789ABCDEF";
    std::string out;
    out.reserve(s.size() * 3);
    for (unsigned char c : s) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            out += c;
        } else {
            out += '%';
            out += kHex[(c >> 4) & 0xf];
            out += kHex[c & 0xf];
        }
    }
    return out;
}

// Parse "http://host:port" → host, port.  Returns false on error.
static bool ParseRemoteUrl(const std::string& url,
                           std::string& host, int& port) {
    std::string s = url;
    if (s.substr(0, 7) == "http://") s = s.substr(7);
    size_t colon = s.rfind(':');
    if (colon == std::string::npos) {
        host = s;
        port = 80;
    } else {
        host = s.substr(0, colon);
        try { port = std::stoi(s.substr(colon + 1)); }
        catch (...) { return false; }
    }
    return !host.empty() && port > 0 && port <= 65535;
}

// Minimal HTTP/1.0 GET.  Returns the response body, or an error string
// prefixed with "[http-error]".
static std::string HttpGet(const std::string& host, int port,
                           const std::string& path) {
    struct addrinfo hints{}, *res = nullptr;
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    std::string port_str = std::to_string(port);
    if (getaddrinfo(host.c_str(), port_str.c_str(), &hints, &res) != 0 || !res)
        return "[http-error] cannot resolve " + host;

    int fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (fd < 0) { freeaddrinfo(res); return "[http-error] socket()"; }
    if (connect(fd, res->ai_addr, res->ai_addrlen) != 0) {
        sock_close(fd); freeaddrinfo(res);
        return "[http-error] connect to " + host + ":" + port_str;
    }
    freeaddrinfo(res);

    std::string req;
    req  = "GET " + path + " HTTP/1.0\r\n";
    req += "Host: " + host + "\r\n";
    req += "Connection: close\r\n\r\n";
    const char* p = req.c_str(); size_t rem = req.size();
    while (rem > 0) { ssize_t n = sock_write(fd, p, rem); if (n <= 0) break; p += n; rem -= n; }

    std::string resp;
    char buf[4096];
    for (;;) { ssize_t n = sock_read(fd, buf, sizeof(buf)); if (n <= 0) break; resp.append(buf, n); }
    sock_close(fd);

    // Split headers / body at \r\n\r\n
    size_t sep = resp.find("\r\n\r\n");
    return sep != std::string::npos ? resp.substr(sep + 4) : resp;
}

// Send GET /core-parser?cmd=<encoded> and return the "result" field value.
static std::string RemoteRunCommand(const std::string& base_url,
                                    const std::string& cmd,
                                    std::string& error_msg) {
    std::string host; int port = 80;
    if (!ParseRemoteUrl(base_url, host, port)) {
        error_msg = "invalid url: " + base_url;
        return "";
    }
    std::string path = "/core-parser?cmd=" + UrlEncode(cmd);
    std::string body = HttpGet(host, port, path);
    if (body.substr(0, 12) == "[http-error]") { error_msg = body; return ""; }
    // Extract {"result":"..."} — reuse JsonGetString
    std::string result = JsonUnescape(JsonGetString(body, "result"));
    if (result.empty() && body.find("\"result\"") == std::string::npos)
        error_msg = "unexpected response: " + body.substr(0, 200);
    return result;
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
                "\"Execute a core-parser command on the LOCAL loaded core/tombstone and return "
                "the text output.  For files on a remote device (e.g. Android via adb forward) "
                "use run_remote_command instead.\\n\\n"
                "IMPORTANT — this is a STATEFUL session.  Loading a core or setting sysroot "
                "persists for all subsequent commands; you do NOT need to reload on every call.\\n\\n"
                "LOADING — choose the right command based on the file type:\\n"
                "  * Tombstone  (TEXT file, name like tombstone_00, tombstone_11, _storage_*tombstone*,\\n"
                "               starts with 'pid:' or 'Cmd line:' when opened as text):\\n"
                "      fake core -t <tombstone> [--sysroot <symbols_dir>] --load\\n"
                "      'fake core' converts the text tombstone into a binary .fakecore, then --load loads it.\\n"
                "      Do NOT use 'fake core' on a binary core/fakecore file — it will fail.\\n\\n"
                "  * Core / fakecore  (BINARY ELF file, name like *.core, *.fakecore, core.*, fake.core.*):\\n"
                "      core <core-file>\\n"
                "      Load directly. Do NOT pass it to 'fake core -t'.\\n\\n"
                "Typical crash-analysis workflow:\\n"
                "  1. Load the file using the correct command above\\n"
                "  2. bt          — print full backtrace of the crashing thread\\n"
                "  3. f <N> [-n]  — inspect frame N: registers + disassembly\\n"
                "  4. map / read  — examine VMA layout or raw memory\\n\\n"
                "Session-management commands:\\n"
                "  fake core -t <tomb> [--sysroot <dir>] [--load]   convert TEXT tombstone to .fakecore;"
                                                                    " --sysroot sets symbol search path;"
                                                                    " --load loads the result immediately\\n"
                "  core <core-file>                                  load a BINARY core or fakecore file\\n"
                "  sysroot <symbols_dir>                             set symbol path for the loaded core\\n"
                "  env                                               show currently loaded core and sysroot\\n"
                "  fake map [option]                                 rebuild fake link_map (run 'help fake' for options)\\n\\n"
                "Thread and backtrace commands:\\n"
                "  bt [PID]                                          backtrace crashing thread, or thread <PID>\\n"
                "  thread [-a|PID]                                   list all threads with state, or switch to thread <PID>\\n"
                "  f <N> [--java|--native]                           inspect frame N (registers + disassembly);"
                                                                    " flags are optional — 'f 0' works\\n"
                "  register                                          dump all registers of the current thread\\n\\n"
                "Memory and disassembly commands:\\n"
                "  map                                               full VMA / memory-map listing\\n"
                "  read <addr> [-e <end_addr>] [--origin]            hexdump memory; --origin shows"
                                                                    " pre-relocation value when sysroot is set\\n"
                "  rd <addr> [-e <end_addr>] [--origin]              alias for read\\n"
                "  vtor <addr>                                       translate virtual address to"
                                                                    " file offset and physical address\\n"
                "  disassemble <addr|symbol> [N] [--origin]           disassemble N instructions; --origin shows original (on-disk) bytes instead of runtime memory\\n\\n"
                "Java / ART commands:\\n"
                "  print <addr> [-x] [-b]                            dump Java object fields at address\\n"
                "  method <addr> [--dexdump] [--oatdump --pc <addr>] dump ArtMethod as dex or oat\\n"
                "  dex                                               list all loaded dex caches\\n"
                "  space                                             list Java heap memory spaces\\n"
                "  class [class_name]                                show fields and methods of a class\\n"
                "  search <class_name>                               find all instances of a class in the heap\\n\\n"
                "Help:\\n"
                "  help [command_name]                               list commands, or show detailed help for one\","
            "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                    "\"command\":{"
                        "\"type\":\"string\","
                        "\"description\":\"The core-parser command to run, "
                            "e.g. \\\"bt\\\", \\\"f 0\\\", \\\"print 0x12345678\\\", \\\"map\\\"\""
                    "}"
                "},"
                "\"required\":[\"command\"]"
            "}"
        "},{"
            "\"name\":\"run_remote_command\","
            "\"description\":"
                "\"Execute a core-parser command on a REMOTE core-parser HTTP daemon "
                "(e.g. running on an Android device via adb forward) and return the output.\\n\\n"
                "The 'url' parameter sets the base URL of the daemon "
                "(e.g. \\\"http://127.0.0.1:8080\\\") and is remembered for all subsequent "
                "calls — you only need to supply it once per session.\\n\\n"
                "Accepts exactly the same commands as run_command.\","
            "\"inputSchema\":{"
                "\"type\":\"object\","
                "\"properties\":{"
                    "\"command\":{"
                        "\"type\":\"string\","
                        "\"description\":\"The core-parser command to run remotely\""
                    "},"
                    "\"url\":{"
                        "\"type\":\"string\","
                        "\"description\":\"Base URL of the remote core-parser daemon, "
                            "e.g. \\\"http://127.0.0.1:8080\\\".  Required on the first call; "
                            "omit on subsequent calls to reuse the last value.\""
                    "}"
                "},"
                "\"required\":[\"command\"]"
            "}"
        "}]}";
    SendResponse(fd, id, kResult);
}

static void HandleToolsCall(int fd, const std::string& id, const std::string& req) {
    std::string tool_name = JsonGetString(req, "name");

    // Narrow to the "arguments" sub-object for all tools.
    size_t args_pos = req.find("\"arguments\"");
    std::string args = args_pos != std::string::npos ? req.substr(args_pos) : req;

    if (tool_name == "run_command") {
        std::string cmd = JsonUnescape(JsonGetString(args, "command"));
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

    } else if (tool_name == "run_remote_command") {
        std::string cmd = JsonUnescape(JsonGetString(args, "command"));
        if (cmd.empty()) {
            SendError(fd, id, -32602, "Missing required argument: command");
            return;
        }
        std::string url = JsonUnescape(JsonGetString(args, "url"));
        if (!url.empty()) s_remote_url = url;
        if (s_remote_url.empty()) {
            SendError(fd, id, -32602, "Missing required argument: url (not yet set)");
            return;
        }
        std::string err;
        std::string output = RemoteRunCommand(s_remote_url, cmd, err);
        if (!err.empty()) {
            SendError(fd, id, -32603, err);
            return;
        }
        std::string result =
            "{\"content\":[{\"type\":\"text\",\"text\":\""
            + JsonEscape(output)
            + "\"}]}";
        SendResponse(fd, id, result);

    } else {
        SendError(fd, id, -32602, "Unknown tool: " + tool_name);
    }
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

    fd_close(stdout_fd);
}
