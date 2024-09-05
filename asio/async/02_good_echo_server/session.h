#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <string>

using boost::asio::ip::tcp;

class CServer;

class CSession : std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& ioc, CServer* server) : sock_(ioc), server_(server) {
        boost::uuids::uuid a_uuid{boost::uuids::random_generator()()};
        uuid_ = boost::uuids::to_string(a_uuid);
    }

public:
    void Start();

    tcp::socket& sock() {
        return sock_;
    }

    std::string& uuid() {
        return uuid_;
    }

private:
    // 读回调函数
    void HandleRead(const boost::system::error_code& error, std::size_t bytes_transfered,
                    std::shared_ptr<CSession> self_shared);
    // 写回调函数
    // 该函数内使用asyncsend可以保证一次性发送完
    void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> self_shared);

private:
    tcp::socket sock_;  // 单独处理客户端读写的socket，其实就是通信套接字
    enum { max_length = 1024 };  // 奇怪的enum->数组长度
    char data_[max_length]{0};  // 接收客户端传递的数据/同时也是回传给客户端的数据
    CServer* server_;
    std::string uuid_;  // 每个session都有uuid，如果有相同的uuid则踢出旧连接
};

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
