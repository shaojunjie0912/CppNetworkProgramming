#include <http_server/logic_system.h>

void LogicSystem::RegGet(std::string const& url, HttpHandler handler) {
    get_handlers_.insert({url, handler});
}

LogicSystem::LogicSystem() {
    RegGet("/get_test", [](std::shared_ptr<HttpConnection> connection) {
        beast::ostream(connection->response_.body()) << "receive get_test req";
    });
}
