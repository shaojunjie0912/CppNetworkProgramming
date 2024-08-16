#include <boost/asio.hpp>
#include <cstddef>
#include <format>
#include <iostream>
#include <string>

#include "boost/asio/ip/tcp.hpp"

using namespace boost;
using std::cout;
using std::endl;

int ClientEndPoint() {
    std::string raw_ip_address{"127.0.0.1"};
    uint16_t port_num{3333};
    boost::system::error_code ec;
    asio::ip::address ip_address{asio::ip::address::from_string(raw_ip_address, ec)};
    // 这就是传说中的
    if (ec.value() != 0) {
        cout << std::format("解析 ip 地址失败，错误码 = {}。错误信息为：{}", ec.value(),
                            ec.message());
        return ec.value();
    }
    asio::ip::tcp::endpoint ep{ip_address, port_num};
    return 0;
}

int main() {
    return 0;
}
