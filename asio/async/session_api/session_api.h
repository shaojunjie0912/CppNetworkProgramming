#pragma once

#include <boost/asio.hpp>
#include <cstring>
#include <iostream>
#include <memory>
#include <queue>
#include <string>

using boost::asio::ip::tcp;

const int kRecvSize = 1024;

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

// 会话类
// 1. 异步写
// 2. 异步读
class CSession {
public:
    CSession(std::shared_ptr<tcp::socket> sock_ptr);
    void Connection(const tcp::endpoint& ep);

public:
    // ----------------------- 异步写 -----------------------
    // Bad
    // 用户发送Hello world -> 在回调之前用户再次发送 Hello world
    // 结果: Hello Hello world world 形成错位
    void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
                          std::shared_ptr<MsgNode>);
    void WriteToSocketErr(const std::string& buf);

    // Good:
    // NOTE: (实际生产环境中通常)采用队列存储消息结点MsgNode
    void WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteToSocket(const std::string& buf);

    // Better:
    // NOTE: (公司实际采用这种简单方式)尽量避免多次调用回调函数
    void WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteAllToSocket(const std::string& buf);

public:
    // ----------------------- 异步读 -----------------------
    // Good: 部分读
    // NOTE: (公司实际采用这种部分读方式)保证读取效率
    void ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void ReadFromSocket();
    // Aslo Good: 一次性读
    void ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void ReadAllFromSocket();

private:
    std::shared_ptr<tcp::socket> sock_ptr_;
    std::shared_ptr<MsgNode> send_node_;               // 发送消息结点
    std::shared_ptr<MsgNode> recv_node_;               // 接收消息结点
    std::queue<std::shared_ptr<MsgNode>> send_queue_;  // 存储消息结点的队列
    bool send_pending_;  // true/false: 队列中是否还有消息结点
    bool recv_pending_;
};
