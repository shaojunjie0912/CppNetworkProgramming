#include <boost/asio.hpp>
#include <cstddef>

using boost::asio::ip::tcp;

class CSession {
public:
    CSession(boost::asio::io_context& ioc) : sock_(ioc) {}

public:
    void Start();

    tcp::socket& sock() {
        return sock_;
    }

private:
    // 读回调函数
    void HandleRead(const boost::system::error_code& error, std::size_t bytes_transfered);
    // 写回调函数
    // 该函数内使用asyncsend可以保证一次性发送完
    void HandleWrite(const boost::system::error_code& error);

private:
    tcp::socket sock_;  // 单独处理客户端读写的socket，其实就是通信套接字
    enum { max_length = 1024 };  // 奇怪的enum->数组长度
    char data_[max_length]{0};  // 接收客户端传递的数据/同时也是回传给客户端的数据
};

class CServer {
public:
    CServer(boost::asio::io_context& ioc, uint16_t port);

private:
    void StartAccept();  // 监听连接
    void HandleAccept(CSession* new_session, const boost::system::error_code& error);
    boost::asio::io_context& ioc_;  // io_context 不能被复制
    tcp::acceptor acceptor_;        // 监听套接字
};
