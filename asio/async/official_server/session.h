#include <boost/asio.hpp>
#include <cstddef>

using boost::asio::ip::tcp;

class Session {
public:
    Session(boost::asio::io_context& ioc) : sock_(ioc) {}
    tcp::socket& sock() {
        return sock_;
    }
    void Start();

private:
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transfered);
    void handle_write(const boost::system::error_code& error);
    tcp::socket sock_;
    enum { max_length = 1024 };  // 奇怪的enum->数组长度
    char _data[max_length];
};
