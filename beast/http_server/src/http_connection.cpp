#include <http_server/http_connection.h>
#include <linux/falloc.h>

#include <cstddef>
#include <exception>

#include "boost/beast/core/error.hpp"
#include "boost/beast/core/ostream.hpp"
#include "boost/beast/http.hpp"
#include "boost/beast/http/field.hpp"
#include "boost/beast/http/status.hpp"
#include "boost/beast/http/verb.hpp"
#include "boost/beast/http/write.hpp"
#include "boost/core/ignore_unused.hpp"

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket)) {}

void HttpConnection::Start() {
    auto self{shared_from_this()};  // 1. 引用计数+1防止回调多次析构 2. lambda捕获
    http::async_read(socket_, buffer_, request_,
                     [self](beast::error_code ec, std::size_t bytes_transfered) {
                         try {
                             if (ec) {
                                 std::cerr << std::format("http read err is: {}\n", ec.what());
                                 return;
                             }
                             // 忽略已经发送的字节数，直接处理请求
                             boost::ignore_unused(bytes_transfered);  // http 服务器不需要粘包处理
                             self->HandleReq();
                             self->CheckDeadline();  // 超时检测
                         } catch (std::exception& e) {
                             std::cerr << std::format("exception is: {}\n", e.what());
                         }
                     });
}

void HttpConnection::HandleReq() {
    // 设置版本
    response_.version(request_.version());
    // 设置短连接
    response_.keep_alive(false);
    if (request_.method() == http::verb::get) {
        bool success{LogicSystem::GetInstance()->HandleGet(request_.target(), shared_from_this())};
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\n";
            WriteResponse();
            return;
        }
        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        WriteResponse();
        return;
    }
}

void HttpConnection::WriteResponse() {
    auto self{shared_from_this()};
    response_.content_length(response_.body().size());  // 粘包处理？
    http::async_write(socket_, response_,
                      [self](beast::error_code ec, std::size_t bytes_transfered) {
                          auto rc1 = self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                          auto rc2 = self->deadline_.cancel();
                      });
}

void HttpConnection::CheckDeadline() {
    auto self{shared_from_this()};
    deadline_.async_wait([self](beast::error_code ec) {
        if (!ec) {
            auto rc = self->socket_.close(ec);
        }
    });
}
