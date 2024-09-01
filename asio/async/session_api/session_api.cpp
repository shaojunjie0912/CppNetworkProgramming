#include "session_api.h"

#include <functional>

CSession::CSession(std::shared_ptr<tcp::socket> sock_ptr)
    : sock_ptr_(sock_ptr), send_pending_(false), recv_pending_(false) {}

void CSession::Connection(const tcp::endpoint& ep) {
    sock_ptr_->connect(ep);
}

// async_write_some的回调函数如何执行？
// 1. 调用async_write_some, 提交异步操作(不影响主线程)
// 2. 后台进行数据写操作
// 3. 异步写操作完成，系统通知 io_service
// 4. io_service 调用 回调函数(此时会获得已经发送的字节数)

void CSession::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
                                std::shared_ptr<MsgNode>) {
    if (ec.value() != 0) {
        std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
        return;
    }
    // 思考：要求发送的消息长度为total_len=10字节，
    // 为什么返回的回调函数中的bytes_transferred=5字节呢？
    // async 本次发送的数据+消息当前长度<消息总长度，则继续发送
    if (bytes_transferred + send_node_->cur_len_ < send_node_->total_len_) {
        sock_ptr_->async_write_some(
            // buffer 长度构造要同步变化
            boost::asio::buffer(send_node_->msg_ + send_node_->cur_len_,
                                send_node_->total_len_ - send_node_->cur_len_),
            std::bind(&CSession::WriteCallBackErr, this, std::placeholders::_1,
                      std::placeholders::_2, send_node_));
    }
}

void CSession::WriteToSocketErr(const std::string& buf) {
    send_node_ = std::make_shared<MsgNode>(buf.c_str(), buf.size());
    // 1. 神奇的函数绑定 参数 3->2，以适配async_write_some回调函数格式
    // 2. 也可以使用lambda
    // sock_ptr_->async_write_some(boost::asio::buffer(send_node_->msg_, send_node_->total_len_),
    //                             std::bind(&CSession::WriteCallBackErr, this,
    //                             std::placeholders::_1,
    //                                       std::placeholders::_2, send_node_));
    sock_ptr_->async_write_some(
        boost::asio::buffer(send_node_->msg_, send_node_->total_len_),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            WriteCallBackErr(ec, bytes_transferred, send_node_);
        });
}

void CSession::WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    // ec值不为0说明出错
    if (ec.value() != 0) {
        std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
        return;
    }
    auto& send_data{send_queue_.front()};
    send_data->cur_len_ += bytes_transferred;
    if (send_data->cur_len_ < send_data->total_len_) {
        sock_ptr_->async_write_some(
            boost::asio::buffer(send_data->msg_ + send_data->cur_len_,
                                send_data->total_len_ - send_data->cur_len_),
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                WriteCallBack(ec, bytes_transferred);
            });
        return;
    }
    send_queue_.pop();
    // 如果消息结点队列为空
    if (send_queue_.empty()) {
        send_pending_ = false;
    }
    // 如果消息结点队列不为空
    else {
        auto& send_data{send_queue_.front()};
        sock_ptr_->async_write_some(
            boost::asio::buffer(send_data->msg_, send_data->total_len_),
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                WriteCallBack(ec, bytes_transferred);
            });
    }
}

void CSession::WriteToSocket(const std::string& buf) {
    send_queue_.emplace(new MsgNode(buf.c_str(), buf.size()));
    if (send_pending_) {  // 若还在发送则->回调函数
        return;
    }
    // 如果没有未发送完数据，则调用异步发送
    sock_ptr_->async_write_some(
        boost::asio::buffer(buf),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            WriteCallBack(ec, bytes_transferred);
        });
    // 队列中有数据
    send_pending_ = true;
}

void CSession::WriteAllCallBack(const boost::system::error_code& ec,
                                std::size_t bytes_transferred) {
    if (ec.value() != 0) {
        std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
        return;
    }
    // 使用 async_send 时，
    // 当回调函数执行时，说明队列第一个消息结点已经发送完了
    send_queue_.pop();
    // 如果消息结点队列为空
    if (send_queue_.empty()) {
        send_pending_ = false;
    }
    // 如果消息结点队列不为空
    else {
        auto& send_data{send_queue_.front()};
        // 第一次发送时, cur_len_=0, 因此不需要加地址偏移
        // NOTE: 一般情况下，都需要封装一个MsgNode并考虑地址偏移
        sock_ptr_->async_write_some(
            boost::asio::buffer(send_data->msg_, send_data->total_len_),
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                WriteCallBack(ec, bytes_transferred);
            });
    }
}

void CSession::WriteAllToSocket(const std::string& buf) {
    send_queue_.emplace(new MsgNode(buf.c_str(), buf.size()));
    if (send_pending_) {
        return;  // 若还在发送则->回调函数
    }
    // 如果没有未发送完数据，则调用异步发送
    sock_ptr_->async_send(boost::asio::buffer(buf), [this](const boost::system::error_code& ec,
                                                           std::size_t bytes_transferred) {
        WriteCallBack(ec, bytes_transferred);
    });
    // 队列中有数据
    send_pending_ = true;
}

void CSession::ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    recv_node_->cur_len_ += bytes_transferred;
    if (recv_node_->cur_len_ < recv_node_->total_len_) {
        sock_ptr_->async_read_some(
            boost::asio::buffer(recv_node_->msg_ + recv_node_->cur_len_,
                                recv_node_->total_len_ - recv_node_->cur_len_),
            [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
                ReadCallBack(ec, bytes_transferred);
            });
    } else {
        send_pending_ = false;
    }
}

// 为什么接收消息不需要像发送消息一样采用队列
void CSession::ReadFromSocket() {
    if (recv_pending_) {
        return;  // 若还在接收则->回调函数
    }
    recv_node_ = std::make_shared<MsgNode>(kRecvSize);  // 创建一个接收消息结点(长度1024)
    sock_ptr_->async_read_some(
        boost::asio::buffer(recv_node_->msg_, recv_node_->total_len_),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            ReadCallBack(ec, bytes_transferred);
        });
    // 下次读发现是true就不会调用异步读
    // 而是在上一次的回调函数中进行
    // 只有当回调函数结束后才将recv_pending_置为false
    recv_pending_ = true;
}

void CSession::ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred) {
    recv_node_->cur_len_ += bytes_transferred;
    send_pending_ = false;
}

void CSession::ReadAllFromSocket() {
    if (recv_pending_) {
        return;  // 若还在接收则->回调函数
    }
    recv_node_ = std::make_shared<MsgNode>(kRecvSize);  // 创建一个接收消息结点(长度1024)
    sock_ptr_->async_receive(
        boost::asio::buffer(recv_node_->msg_, recv_node_->total_len_),
        [this](const boost::system::error_code& ec, std::size_t bytes_transferred) {
            ReadCallBack(ec, bytes_transferred);
        });
    recv_pending_ = true;
}
