#include <locale.h>
#include <netdb.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <format>
#include <iostream>
#include <string_view>

template <typename... Args>
void Print(const std::string_view fmt_str, Args&&... args) {
    auto fmt_args{std::make_format_args(args...)};
    std::string outstr{std::vformat(fmt_str, fmt_args)};
    fputs(outstr.c_str(), stdout);
}
int main() {
    setlocale(LC_ALL, "zh_CN.UTF-8");
    struct addrinfo* addrinfo;
    int res = getaddrinfo("127.0.0.1", "80", NULL, &addrinfo);
    if (res != 0) {
        perror("getaddrinfo");
        return 1;
    }
    Print("{} {}\n", addrinfo->ai_socktype, addrinfo->ai_protocol);
    int sockfd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
    if (sockfd == -1) {
        Print("erron: {}", errno);
        return 1;
    }
}
