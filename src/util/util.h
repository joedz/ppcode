#pragma once 
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>

namespace ppcode {

    //获取本线程id
    pid_t GetThreadId();

    //获取系统中一页的大小
    size_t GetPageSize();

}