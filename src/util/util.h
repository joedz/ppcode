#pragma once
#include <cxxabi.h>
#include <pthread.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>
#include <vector>

namespace ppcode {

//获取本线程id
pid_t GetThreadId();

// 获取协程id
uint32_t GetFiberId();

//获取系统中一页的大小
size_t GetPageSize();

// 获取 T类型的变量类型
template <class T>
const char* GetTypeName() {
    static char* s_strType =
        abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
    return s_strType;
}

// 获取当前函数的调用栈
void Backtrace(std::vector<std::string>& bt, int size = 64, int skip = 1);
// 获取当前的函数调用栈信息的字符串
std::string BacktraceToString(int size = 64, int skip = 2,
                              const std::string& prefix = "");


// 获取当前时间的毫秒
uint64_t GetCurrentMS();

 //获取当前时间的微秒
uint64_t GetCurrentUS();


}  // namespace ppcode