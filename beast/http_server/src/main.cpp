#include <http_server/common.h>
#include <http_server/cserver.h>

#include <csignal>

int main() {
    try {
        uint16_t port{8080};
        asio::io_context ioc;
        // signal interrupt: 终端按下 Ctrl C
        // signal terminate: 程序发送 kill 命令
        asio::signal_set signals{ioc, SIGINT, SIGTERM};
        signals.async_wait([&ioc](boost::system::error_code const& ec, int signal_number) {
            if (ec) {
                return;
            }
            ioc.stop();
        });
        std::make_shared<CServer>(ioc, port)->Start();
        std::cout << std::format("Http Server listen on port: {}\n", port);
        ioc.run();

    } catch (std::exception& e) {
        std::cerr << std::format("exception is: {}\n", e.what());
    }
    return 0;
}
