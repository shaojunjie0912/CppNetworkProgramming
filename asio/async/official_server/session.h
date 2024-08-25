#include <boost/asio.hpp>
#include <cstddef>

using boost::asio::ip::tcp;

class Session {
public:
    Session(boost::asio::io_context& ioc) : sock_(ioc) {}

public:
    void Start();

    tcp::socket& sock() {
        return sock_;
    }

private:
    // 读回调函数
    void HandleRead(const boost::system::error_code& error, std::size_t bytes_transfered);
    // 写回调函数
    void HandleWrite(const boost::system::error_code& error);

private:
    tcp::socket sock_;  // 单独处理客户端读写的socket，其实就是通信套接字
    enum { max_length = 1024 };  // 奇怪的enum->数组长度
    char data_[max_length]{};  // 接收客户端传递的数据/同时也是回传给客户端的数据
};

class Server {
public:
    Server(boost::asio::io_context& ioc, uint16_t port);

private:
    void StartAccept();
    void HandleAccept(Session* new_session, const boost::system::error_code& error);
    boost::asio::io_context& ioc_;  // io_context 不能被复制
    tcp::acceptor acceptor_;
};
