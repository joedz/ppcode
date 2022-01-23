#include "util.h"

#include <cxxabi.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <time.h>
#include <sys/time.h>
#include <execinfo.h>

#include "../log.h"

namespace ppcode {

static Logger::ptr g_logger = LOG_ROOT();

//获取本线程id
pid_t GetThreadId() { return syscall(SYS_gettid); }

// 获取协程id
uint32_t GetFiberId() { return 0; }

//获取系统中一页的大小
size_t GetPageSize() { return syscall(_SC_PAGE_SIZE); }

// 获取当前函数的调用栈
void Backtrace(std::vector<std::string>& bt, int size, int skip) {
    void** array = (void**)malloc((sizeof(void*) * size));
    size_t s = ::backtrace(array, size);

    char** strings = backtrace_symbols(array, s);
    if (strings == NULL) {
        LOG_ERROR(g_logger) << "backtrace_synbols error";
        return;
    }

    for (size_t i = skip; i < s; ++i) {
        bt.push_back(strings[i]);
    }

    free(strings);
    free(array);
}
// 获取当前的函数调用栈信息的字符串
std::string BacktraceToString(int size, int skip,
                              const std::string& prefix ) {
    std::vector<std::string> bt;
    Backtrace(bt, size, skip);
    std::stringstream ss;
    for (size_t i = 0; i < bt.size(); ++i) {
        ss << prefix << bt[i] << std::endl;
    }
    return ss.str();
}

// 获取当前时间的毫秒
uint64_t GetCurrentMS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000ul + tv.tv_usec / 1000;
}

//获取当前时间的微秒
uint64_t GetCurrentUS() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000 * 1000ul + tv.tv_usec;
}

}  // namespace ppcode
