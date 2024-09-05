#include "session.h"

#include <cstring>
#include <iostream>

using boost::asio::ip::tcp;
using std::cout;
using std::endl;

// 隐患：
// 假设服务器在调用async_write时客户端断开连接
// 则async_write的回调函数HandleWrite中会delete
// 而检测到对端关闭，则服务器又会调用async_read_some的回调函数HandleRead再次delete

// 直接先实现最重要的，假设HandleRead和HandleWrite都已经实现
// Start 通过 async_read_some 发起非阻塞的读取操作，等待客户端发送数据
// 如果有数据到达，则调用 HandleRead 处理
void CSession::Start() {
    sock_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
            HandleRead(error, bytes_transfered);
        });
}

// 好像是读取到数据后，将数据存储到data_中后才会调用HandleRead
// HandleRead 又通过 async_write 将数据回传给客户端
void CSession::HandleRead(const boost::system::error_code& error, std::size_t bytes_transfered) {
    if (!error) {
        cout << "CServer receive data is: " << data_ << endl;
        boost::asio::async_write(sock_, boost::asio::buffer(data_, bytes_transfered),
                                 [this](const boost::system::error_code& error,
                                        std::size_t /*bytes_transfered*/) { HandleWrite(error); });
    } else {
        std::cout << "read error" << std::endl;
        delete this;  // 隐患
    }
}

// 然后回传结束后调用 HandleWrite 再从客户端读数据
void CSession::HandleWrite(const boost::system::error_code& error) {
    if (!error) {
        memset(data_, 0, max_length);
        sock_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
                HandleRead(error, bytes_transfered);
            });
    } else {
        std::cout << "write error " << error.value() << std::endl;
        delete this;  // 隐患
    }
}

CServer::CServer(boost::asio::io_context& ioc, uint16_t port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
    std::cout << "CServer start success, on port: " << port << std::endl;
    StartAccept();
}

void CServer::StartAccept() {
    CSession* new_session{new CSession(ioc_)};   // 创建一个新 session
    acceptor_.async_accept(new_session->sock(),  // 使用 session 中的 socket
                           [this, new_session](const boost::system::error_code& error) {
                               return HandleAccept(new_session, error);
                           });
}

void CServer::HandleAccept(CSession* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->Start();
    } else {
        delete new_session;
    }
    StartAccept();
}
