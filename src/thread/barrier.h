#pragma once

#include <pthread.h>

namespace ppcode {

class Barrier{
public:
    Barrier(int count);
    ~Barrier();

    // 阻塞到这里的进程, 数量为barrier规定的数量
    // [return] 是否为选中的主线程
    bool wait();

private:
    pthread_barrier_t m_barrier;
    int count;
};

}