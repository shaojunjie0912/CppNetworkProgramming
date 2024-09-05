#include <exception>

#include "cserver.h"
#include "csession.h"

int main() {
    try {
        boost::asio::io_context ioc;
        using namespace std;
        CServer s{ioc, 8080};
        ioc.run();
    } catch (std::exception const& e) {
    }
}
