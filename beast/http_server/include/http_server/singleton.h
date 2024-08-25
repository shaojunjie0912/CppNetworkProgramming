#pragma once

#include <iostream>
#include <memory>
#include <mutex>

template <typename T>
class Singleton {
protected:  // 神奇的protected？CRTP？
    Singleton() = default;
    Singleton(Singleton<T> const&) = delete;
    Singleton& operator=(Singleton<T> const&) = delete;

    inline static std::shared_ptr<T> instance_{nullptr};

public:
    static std::shared_ptr<T> GetInstance() {
        static std::once_flag s_flag;
        // 为什么不适用make_shared而是new？
        // 因为make_shared无法访问protected的构造函数
        std::call_once(s_flag, [&] { instance_ = std::shared_ptr<T>(new T); });
        return instance_;
    }

    void PrintAddress() {
        std::cout << instance_.get() << std::endl;
    }

    ~Singleton() {
        std::cout << "this is singleton destruct" << std::endl;
    }
};
