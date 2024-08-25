#pragma once

#include <http_server/common.h>
#include <http_server/http_connection.h>

#include <unordered_map>

class HttpConnection;

using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem();
    bool HandleGet(std::string const& path, std::shared_ptr<HttpConnection> conn);
    void RegGet(std::string const& url, HttpHandler handler);

private:
    LogicSystem();
    std::unordered_map<std::string, HttpHandler> post_handlers_;
    std::unordered_map<std::string, HttpHandler> get_handlers_;
};
