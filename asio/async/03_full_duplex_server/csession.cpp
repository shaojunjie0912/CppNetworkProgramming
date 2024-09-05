
#include "csession.h"

#include <cstring>
#include <iostream>
#include <mutex>

#include "cserver.h"

using boost::asio::ip::tcp;
using std::cout;
using std::endl;

CSession::CSession(boost::asio::io_context& ioc, CServer* server) : sock_(ioc), server_(server) {
    boost::uuids::uuid a_uuid{boost::uuids::random_generator()()};
    uuid_ = boost::uuids::to_string(a_uuid);
}

void CSession::Start() {
    sock_.async_read_some(
        boost::asio::buffer(data_, max_length),
        [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
            HandleRead(error, bytes_transfered,
                       shared_from_this());  // 这里不能用裸指针生成shared_ptr
        });
}

// 发送接口里判断发送队列是否为空，如果不为空说明有数据未发送完，需要将数据放入队列，然后返回。
// 如果发送队列为空，则说明当前没有未发送完的数据，将要发送的数据放入队列并调用async_write函数发送数据。
void CSession::Send(char* msg, int max_length) {
    std::lock_guard<std::mutex> lk{send_mtx_};
    bool pending{false};
    if (send_queue_.size() > 0) {
        pending = true;
    }
    send_queue_.push(std::make_shared<MsgNode>(msg, max_length));
    if (pending) {
        return;
    }
    boost::asio::async_write(
        sock_, boost::asio::buffer(msg, max_length),
        [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
            HandleWrite(error, shared_from_this());
        });
}

void CSession::HandleWrite(const boost::system::error_code& error,
                           std::shared_ptr<CSession> self_shared) {
    if (!error) {
        std::lock_guard<std::mutex> lk{send_mtx_};
        send_queue_.pop();  // 这里队列首元素已经被发完了，因此才进回调
        if (!send_queue_.empty()) {
            auto msg_node{send_queue_.front()};
            boost::asio::async_write(
                sock_, boost::asio::buffer(msg_node->data_, msg_node->max_len_),
                [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
                    HandleWrite(error, shared_from_this());
                });
        }
    } else {
        std::cout << "write error " << error.value() << std::endl;
        server_->ClearSession(uuid_);
    }
}

void CSession::HandleRead(const boost::system::error_code& error, std::size_t bytes_transfered,
                          std::shared_ptr<CSession> self_shared) {
    if (!error) {
        cout << "CServer receive data is: " << data_ << endl;
        Send(data_, bytes_transfered);
        memset(data_, 0, max_length);
        // 全双工通信，在发送的同时也需要同时监听读事件
        sock_.async_read_some(
            boost::asio::buffer(data_, max_length),
            [this](const boost::system::error_code& error, std::size_t bytes_transfered) {
                HandleRead(error, bytes_transfered, shared_from_this());
            });
    } else {
        std::cout << "read error" << std::endl;
        server_->ClearSession(uuid_);
    }
}
