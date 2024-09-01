#include "cserver.h"

#include <iostream>

CServer::CServer(boost::asio::io_context& ioc, uint16_t port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "CServer start success, on port: " << port << std::endl;
    StartAccept();
}

void CServer::StartAccept() {
    auto new_session{std::make_shared<CSession>(ioc_, this)};  // 创建一个新 session
    acceptor_.async_accept(new_session->sock(),                // 使用 session 中的 socket
                           [this, new_session](const boost::system::error_code& error) {
                               return HandleAccept(new_session, error);
                           });
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session,
                           const boost::system::error_code& error) {
    if (!error) {
        new_session->Start();
        sessions_[new_session->uuid()] = new_session;
    } else {
    }
    StartAccept();
}
