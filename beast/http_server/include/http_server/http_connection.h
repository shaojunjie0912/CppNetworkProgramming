#pragma once

#include <http_server/common.h>
#include <http_server/logic_system.h>

class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
    friend class LogicSystem;

public:
    HttpConnection(tcp::socket socket);

public:
    void Start();

private:
    void CheckDeadline();
    void WriteResponse();
    void HandleReq();

private:
    tcp::socket socket_;
    beast::flat_buffer buffer_{8192};
    http::request<http::dynamic_body> request_;    // 请求
    http::response<http::dynamic_body> response_;  // 回复
    asio::steady_timer deadline_{socket_.get_executor(), std::chrono::seconds(60)};
};
