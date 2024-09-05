#pragma once

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <cstdint>
#include <map>
#include <memory>
#include <queue>
#include <string>

#define MAX_LENGTH 1024 * 2
#define HEAD_LENGTH 2

using boost::asio::ip::tcp;

class CServer;

// TLV 消息格式
// 消息id(暂不考虑) + 消息长度 + 消息内容
class MsgNode {
    friend class CSession;

public:
    // 发送消息结点
    MsgNode(char* msg, int max_len) : total_len_(max_len + HEAD_LENGTH), cur_len_(0) {
        data_ = new char[total_len_ + 1];           // 最后一字节用于存储'\0'
        memcpy(data_, &max_len, HEAD_LENGTH);       // 拷贝消息长度->2字节
        memcpy(data_ + HEAD_LENGTH, msg, max_len);  // 拷贝消息体->max_len字节
        data_[total_len_] = '\0';                   // '\0' 最后1字节，方便打印
    }

    // 接收消息结点
    MsgNode(uint16_t max_len) : total_len_(max_len), cur_len_(0) {
        data_ = new char[total_len_ + 1];
    }

    ~MsgNode() {
        delete[] data_;
    }

public:
    void Clear() {
        memset(data_, 0, total_len_);
        cur_len_ = 0;
    }

private:
    int cur_len_;
    int total_len_;
    char* data_;
};

class CSession : public std::enable_shared_from_this<CSession> {
public:
    CSession(boost::asio::io_context& ioc, CServer* server);

public:
    void Start();
    void Send(char* msg, int max_length);

public:
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
    std::queue<std::shared_ptr<MsgNode>> send_queue_;
    std::mutex send_mtx_;
    std::shared_ptr<MsgNode> recv_msg_node_;   // 收到的消息体
    bool b_head_parse_;                        // 头部是否解析完成
    std::shared_ptr<MsgNode> recv_head_node_;  // 收到的头部结构
};
