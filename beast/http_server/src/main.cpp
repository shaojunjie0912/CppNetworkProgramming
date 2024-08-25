#include <http_server/common.h>
#include <http_server/cserver.h>

#include <csignal>

int main() {
    try {
        uint16_t port{8080};
        asio::io_context ioc;
        asio::signal_set signals{ioc, SIGINT, SIGTERM};
        signals.async_wait([&ioc](boost::system::error_code const& ec, int signal_number) {
            if (ec) {
                return;
            }
            ioc.stop();
        });
        std::make_shared<CServer>(ioc, port)->Start();
        ioc.run();

    } catch (std::exception& e) {
        std::cerr << std::format("exception is: {}\n", e.what());
    }
    return 0;
}
