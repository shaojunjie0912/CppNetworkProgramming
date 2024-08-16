#include <boost/asio.hpp>
#include <cstddef>
#include <iostream>
#include <memory>
#include <set>

using boost::asio::ip::tcp;
using std::cout;
using std::endl;

using socket_ptr = std::shared_ptr<tcp::socket>;    // socket 智能指针
std::set<std::shared_ptr<std::thread>> thread_set;  // 线程指针集合

const int max_length = 1024;

void Session(socket_ptr sock_ptr) {
    try {
        for (;;) {
            char data[max_length];
            memset(data, '\0', max_length);
            boost::system::error_code error;
            size_t length = sock_ptr->read_some(boost::asio::buffer(data, max_length), error);
            if (error == boost::asio::error::eof) {
                std::cout << "connection closed by peer" << endl;
                break;
            } else if (error) {
                throw boost::system::system_error(error);
            }
            cout << "receive from " << sock_ptr->remote_endpoint().address().to_string() << endl;
            cout << "receive message is " << data << endl;
            // 回传信息值
            boost::asio::write(*sock_ptr, boost::asio::buffer(data, length));
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
    }
}

void Server(boost::asio::io_context& ioc, uint16_t port) {
    // 创建监听套接字
    tcp::acceptor a{ioc, tcp::endpoint{tcp::v4(), port}};
    while (true) {
        // 创建通信套接字(用与监听套接字一样的上下文初始化)
        socket_ptr sock_ptr{new tcp::socket{ioc}};
        a.accept(*sock_ptr);
        // 创建自定义的处理连接的线程
        auto t{std::make_shared<std::thread>(Session, sock_ptr)};
        thread_set.insert(t);  // 生命周期与全局变量thread_set一样长
    }
}

int main() {
    try {
        boost::asio::io_context ioc;
        Server(ioc, 10086);
        for (auto& t : thread_set) {
            t->join();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception " << e.what() << "\n";
    }
    return 0;
}
