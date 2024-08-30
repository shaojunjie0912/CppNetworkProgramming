#pragma once

#include <http_server/singleton.h>
#include <json/json.h>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <chrono>
#include <cstdint>
#include <format>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;

using boost::asio::ip::tcp;

enum class ErrorCodes {
    Success = 0,
    Error_Json = 1001,  // Json 解析错误
    Rpc_Failed = 1002   // Rpc 请求错误
};
