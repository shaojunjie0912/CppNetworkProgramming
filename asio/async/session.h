#include <boost/asio.hpp>
#include <cstring>
#include <memory>
#include <string>

#include "boost/asio/ip/tcp.hpp"

using boost::asio::ip::tcp;

struct MsgNode {
    // 构造函数-写结点
    MsgNode(const char* msg, int total_len) : total_len_(total_len), cur_len_(0) {
        msg_ = new char[total_len];
        memcpy(msg_, msg, total_len);  // 将msg拷贝进来
    }
    // 构造函数-读结点
    MsgNode(int total_len) : total_len_(total_len), cur_len_(0) {
        msg_ = new char[total_len];
    }
    ~MsgNode() {
        delete[] msg_;
    }

    char* msg_;      // 消息首地址
    int total_len_;  // 消息总长度
    int cur_len_;    // 当前长度
};

class Session {
public:
    Session(std::shared_ptr<tcp::socket> sock_ptr);

    void Connection(const tcp::endpoint& ep);

    // Bad
    // 用户发送Hello world -> 在回调之前用户再次发送 Hello world
    // 结果: Hello Hello world world 形成错位
    void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
                          std::shared_ptr<MsgNode>);
    void WriteToSocketErr(const std::string& buf);
    // Good:

private:
    std::shared_ptr<tcp::socket> sock_ptr_;
    std::shared_ptr<MsgNode> send_node_;
};
