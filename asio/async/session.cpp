#include "session.h"

#include <functional>

void Session::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
                               std::shared_ptr<MsgNode>) {
    // 思考：要求发送的消息长度为total_len=10字节，
    // 为什么返回的回调函数中的bytes_transferred=5字节呢？
    // async 本次发送的数据+消息当前长度<消息总长度，则继续发送
    if (bytes_transferred + send_node_->cur_len_ < send_node_->total_len_) {
        sock_ptr_->async_write_some(
            // buffer 长度构造要同步变化
            boost::asio::buffer(send_node_->msg_ + send_node_->cur_len_,
                                send_node_->total_len_ - send_node_->cur_len_),
            std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1,
                      std::placeholders::_2, send_node_));
    }
}

void Session::WriteToSocketErr(const std::string& buf) {
    send_node_ = std::make_shared<MsgNode>(buf.c_str(), buf.size());
    // 神奇的函数绑定 参数 3->2，以适配async_write_some回调函数格式
    sock_ptr_->async_write_some(boost::asio::buffer(send_node_->msg_, send_node_->total_len_),
                                std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1,
                                          std::placeholders::_2, send_node_));
}
