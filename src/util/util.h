#pragma once
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <cxxabi.h>

namespace ppcode
{

    //获取本线程id
    pid_t GetThreadId();

    //获取系统中一页的大小
    size_t GetPageSize();

    template<class T>
    const char* GetTypeName(){
        static char * s_strType = abi::__cxa_demangle(typeid(T).name(), nullptr, nullptr, nullptr);
        return s_strType; 
    }

} // namespace ppcode