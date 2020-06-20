#pragma once

#include <pthread.h>
#include "mutex.h"

namespace ppcode {

//条件变量
class Condition_variable{
public:
    Condition_variable();
    ~Condition_variable();

    // 必须传入加锁互斥锁
    void wait(Mutex& lock);

    // 必须传进的锁已经加锁的锁
    template<class predicate_type>
    void wati(Mutex& lock, predicate_type pred){
        // 条件变量应在互斥中访问
        while(!pred()){
            wait(lock);
        }
    }

    void timeWait(Mutex& lock, int sec, int mesc);

    // 唤醒一个进程
    void notify_one();
    // 唤醒所有进程
    void notify_all();

    // 获取接口
    pthread_cond_t* native_handle(){ return &m_cond;}
    const pthread_cond_t* native_handle() const{ return &m_cond;}

private:
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
};


}