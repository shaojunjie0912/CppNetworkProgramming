#include <http_server/http_connection.h>
#include <http_server/logic_system.h>

LogicSystem::LogicSystem() {
    // 注册请求
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req\n";
        int i{0};
        for (auto const& [key, val] : connection->get_params_) {
            ++i;
            beast::ostream(connection->response_.body())
                << "param" << i << " key is " << key << " value is " << val << '\n';
        }
    });
}

bool LogicSystem::HandleGet(std::string const& path, std::shared_ptr<HttpConnection> conn) {
    // 如果url没有注册过则返回404错误
    if (get_handlers_.find(path) == get_handlers_.end()) {
        return false;
    }
    // get_handlers_[path]返回回调函数，输入参数为conn
    get_handlers_[path](conn);
    return true;
}

void LogicSystem::RegGet(std::string const& url, HttpHandler handler) {
    get_handlers_.insert({url, handler});
}

void LogicSystem::RegPost(std::string const& url, HttpHandler handler) {
    post_handlers_.insert({url, handler});
}
