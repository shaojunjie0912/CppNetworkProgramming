#include "session.h"

#include <cstring>
#include <iostream>

using boost::asio::ip::tcp;
using std::cout;
using std::endl;

void Session::Start() {
    sock_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
            HandleRead(error, bytes_transfered);
        });
}

void Session::HandleRead(const boost::system::error_code& error, std::size_t bytes_transfered) {
    if (!error) {
        cout << "Server receive data is: " << data_ << endl;
        boost::asio::async_write(sock_, boost::asio::buffer(data_, bytes_transfered),
                                 [this](const boost::system::error_code& error,
                                        std::size_t /*bytes_transfered*/) { HandleWrite(error); });
    } else {
        std::cout << "read error" << std::endl;
        delete this;
    }
}

void Session::HandleWrite(const boost::system::error_code& error) {
    if (!error) {
        memset(data_, 0, max_length);
        sock_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
                HandleRead(error, bytes_transfered);
            });
    }
}

Server::Server(boost::asio::io_context& ioc, uint16_t port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
    StartAccept();
}

void Server::StartAccept() {
    Session* new_session{new Session(ioc_)};
    acceptor_.async_accept(new_session->sock(),
                           [this, new_session](const boost::system::error_code& error) {
                               return HandleAccept(new_session, error);
                           });
}

void Server::HandleAccept(Session* new_session, const boost::system::error_code& error) {
    if (!error) {
        new_session->Start();
    } else {
        delete new_session;
    }
    StartAccept();
}
