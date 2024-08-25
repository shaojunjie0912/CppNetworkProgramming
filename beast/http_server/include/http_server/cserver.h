#pragma once

#include <http_server/common.h>
#include <http_server/http_connection.h>

class CServer : public std::enable_shared_from_this<CServer> {
public:
    CServer(asio::io_context& ioc, const uint16_t port);
    void Start();

private:
    asio::io_context& ioc_;
    tcp::acceptor acceptor_;
    tcp::socket socket_;
};
