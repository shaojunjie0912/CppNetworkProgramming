#include <http_server/cserver.h>

#include <exception>

CServer::CServer(asio::io_context& ioc, const uint16_t port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), socket_(ioc) {}

void CServer::Start() {
    auto self{shared_from_this()};
    acceptor_.async_accept(socket_, [self](beast::error_code ec) {
        try {
            // 出错放弃此连接，继续监听其他连接
            if (ec) {
                self->Start();
                return;
            }
            // 创建新连接，并创建 HttpConnection 类管理连接
            // 这里临时对象的创建没有问题因为内部shared_from_this
            std::make_shared<HttpConnection>(std::move(self->socket_))->Start();
            // 继续监听
            self->Start();
        } catch (std::exception& e) {
        }
    });
}
