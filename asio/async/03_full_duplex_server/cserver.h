#pragma once

#include <boost/asio.hpp>

#include "csession.h"

class CServer {
public:
    CServer(boost::asio::io_context& ioc, uint16_t port);

public:
    void ClearSession(std::string uuid) {
        sessions_.erase(uuid);
    }

private:
    void StartAccept();  // 监听连接
    void HandleAccept(std::shared_ptr<CSession> new_session,
                      const boost::system::error_code& error);
    boost::asio::io_context& ioc_;  // io_context 不能被复制
    tcp::acceptor acceptor_;        // 监听套接字
    std::map<std::string, std::shared_ptr<CSession>> sessions_;
};
