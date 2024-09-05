#include <exception>

#include "boost/asio/io_context.hpp"
#include "session.h"

int main() {
    try {
        boost::asio::io_context ioc;
        using namespace std;
        CServer s{ioc, 8080};
        ioc.run();
    } catch (std::exception const& e) {
    }
}
