#pragma once

#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstddef>
#include <map>
#include <memory>
#include <queue>
#include <string>

using boost::asio::ip::tcp;

class CServer;

class MsgNode {
    friend class CSession;

public:
    MsgNode(char* msg, int max_len) {
        data_ = new char[max_len];
        memcpy(data_, msg, max_len);
    }
    ~MsgNode() {
        delete[] data_;
    }

private:
    int cur_len_;
    int max_len_;
    char* data_;
};

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

    void Send(char* msg, int max_length);

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
};
