#pragma once

#include <http_server/common.h>

#include <unordered_map>

// LogicSystem 类和 HttpConnection 类存在互相引用
// 解决方法：在二者.cpp文件中包含对方，头文件中以提前声明或者类中友元类方式
class HttpConnection;

using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicSystem : public Singleton<LogicSystem> {
    friend class Singleton<LogicSystem>;

public:
    ~LogicSystem() {}

public:
    bool HandleGet(std::string const& path, std::shared_ptr<HttpConnection> conn);
    void RegGet(std::string const& url, HttpHandler handler);
    void RegPost(std::string const& url, HttpHandler handler);

private:
    LogicSystem();
    std::unordered_map<std::string, HttpHandler> post_handlers_;
    std::unordered_map<std::string, HttpHandler> get_handlers_;
};
