#include <http_server/http_connection.h>
#include <http_server/logic_system.h>

#include <cassert>
#include <cctype>
#include <cstddef>
#include <exception>

HttpConnection::HttpConnection(tcp::socket socket) : socket_(std::move(socket)) {}

void HttpConnection::Start() {
    auto self{shared_from_this()};  // 1. 引用计数+1防止回调多次析构 2. lambda捕获
    http::async_read(socket_, buffer_, request_,
                     [self](beast::error_code ec, std::size_t bytes_transfered) {
                         try {
                             if (ec) {
                                 std::cerr << std::format("http read err is: {}\n", ec.what());
                                 return;
                             }
                             // 忽略已经发送的字节数，直接处理请求
                             boost::ignore_unused(bytes_transfered);  // http 服务器不需要粘包处理
                             self->HandleReq();                       // 处理读请求
                             self->CheckDeadline();                   // 超时检测
                         } catch (std::exception& e) {
                             std::cerr << std::format("exception is: {}\n", e.what());
                         }
                     });
}

unsigned char ToHex(unsigned char num) {
    unsigned char ch = num > 9 ? num + 55 : num + 48;
    return ch;
}

unsigned char FromHex(unsigned char ch) {
    unsigned char num;
    if (ch >= 'A' && ch <= 'Z') {
        num = ch - 'A' + 10;
    } else if (ch >= 'a' && ch <= 'z') {
        num = ch - 'a' + 10;
    } else if (ch >= '0' && ch <= '9') {
        num = ch - '0';
    } else {
        assert(0);
    }
    return num;
}

// URL 编码
std::string UrlEncode(std::string const& str) {
    std::string str_temp;
    for (auto const& ch : str) {
        // 判断是否仅有数字/字母构成
        if (isalnum(ch) || (ch == '-') || (ch == '_') || (ch == '.') || (ch == '~')) {
            str_temp += ch;
        }
        // 空字符转为 '+'
        else if (ch == ' ') {
            str_temp += '+';
        }
        // 其他字符需要提前加%并且高四位和低四位分别转为16进制
        else {
            str_temp += '%';
            str_temp += ToHex(ch >> 4);
            str_temp += ToHex(ch & 0x0F);
        }
    }
    return str_temp;
}

std::string UrlDecode(const std::string& str) {
    std::string strTemp;
    size_t length = str.size();
    for (size_t i = 0; i < length; i++) {
        // 还原+为空
        if (str[i] == '+')
            strTemp += ' ';
        // 遇到%将后面的两个字符从16进制转为char再拼接
        else if (str[i] == '%') {
            assert(i + 2 < length);
            unsigned char high = FromHex(str[++i]);
            unsigned char low = FromHex(str[++i]);
            strTemp += high * 16 + low;
        } else
            strTemp += str[i];
    }
    return strTemp;
}

void HttpConnection::PreParseGetParam() {
    // 提取 URI
    auto uri = request_.target();
    // 查找查询字符串的开始位置（即 '?' 的位置）
    auto query_pos = uri.find('?');
    if (query_pos == std::string::npos) {
        get_url_ = uri;
        return;
    }

    get_url_ = uri.substr(0, query_pos);
    std::string query_string = uri.substr(query_pos + 1);
    std::string key;
    std::string value;
    size_t pos = 0;
    while ((pos = query_string.find('&')) != std::string::npos) {
        auto pair = query_string.substr(0, pos);
        if (size_t eq_pos = pair.find('='); eq_pos != std::string::npos) {
            key = UrlDecode(pair.substr(0, eq_pos));  // 假设有 url_decode 函数来处理URL解码
            value = UrlDecode(pair.substr(eq_pos + 1));
            get_params_[key] = value;
        }
        query_string.erase(0, pos + 1);
    }
    // 处理最后一个参数对（如果没有 & 分隔符）
    if (!query_string.empty()) {
        if (size_t eq_pos = query_string.find('='); eq_pos != std::string::npos) {
            key = UrlDecode(query_string.substr(0, eq_pos));
            value = UrlDecode(query_string.substr(eq_pos + 1));
            get_params_[key] = value;
        }
    }
}

void HttpConnection::HandleReq() {
    // 设置版本
    response_.version(request_.version());
    // 设置短连接
    response_.keep_alive(false);
    if (request_.method() == http::verb::get) {
        PreParseGetParam();
        // 调用逻辑层处理请求(参数1: url, 参数2: connection)
        bool success{LogicSystem::GetInstance()->HandleGet(get_url_, shared_from_this())};
        if (!success) {
            response_.result(http::status::not_found);
            response_.set(http::field::content_type, "text/plain");
            beast::ostream(response_.body()) << "url not found\n";
            WriteResponse();
            return;
        }
        // 如果底层LogicSystem处理成功，则返回成功状态
        response_.result(http::status::ok);
        response_.set(http::field::server, "GateServer");
        // 发送该包
        WriteResponse();
        return;
    }
}

void HttpConnection::WriteResponse() {
    auto self{shared_from_this()};
    response_.content_length(response_.body().size());  // 粘包处理？
    http::async_write(socket_, response_,
                      [self](beast::error_code ec, std::size_t bytes_transfered) {
                          auto rc1 = self->socket_.shutdown(tcp::socket::shutdown_send, ec);
                          auto rc2 = self->deadline_.cancel();
                      });
}

void HttpConnection::CheckDeadline() {
    auto self{shared_from_this()};
    deadline_.async_wait([self](beast::error_code ec) {
        if (!ec) {
            auto rc = self->socket_.close(ec);
        }
    });
}
