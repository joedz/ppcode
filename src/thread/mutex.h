#pragma once

#include "../util/noncopyable.h"
#include "scopedlock.h"

#include <pthread.h>
#include <memory>


namespace ppcode {

// 互斥锁属性
class Mutex_Attributes{
public:

    Mutex_Attributes();
    ~Mutex_Attributes();

    // 因为pthread锁缺省设置为独立进程间不共享，本接口默认设置独立进程间共享
    void setProcessShared(int pshared = PTHREAD_PROCESS_SHARED);
    // 判断是狗为进程间共享互斥锁
    bool isProcessShared();

    // 因为pthread锁缺省设置为持有互斥锁进程间终止时不采取任何行动，本接口默认设置为相反操作
    void setRobust(int robust = PTHREAD_MUTEX_ROBUST);
    // 判断是否在互斥锁进程终止时没有解锁，在其他进程对相同互斥锁加锁时，返回EOWBEDEAD
    bool isRobust();

    /**
     * 设置互斥锁属性
     * PTHREAD_MUTEX_NORAMAL    不提供任何错误检车或死锁检查
     * PTHREAD_MUTEX_ERRORCHECK 提供互斥量错误类型检查
     * PTHREAD_MUTEX_RECURSIVE  递归锁
     * PTHREAD_MUTEX_DEFAULT    默认锁
     * 默认设置成递归锁
     * */
    void setMutexType(int type = PTHREAD_MUTEX_RECURSIVE);
    int getMutexType();

    pthread_mutexattr_t* native_handle(){ return &m_mutex_attr;}
    const pthread_mutexattr_t* native_handle() const{ return &m_mutex_attr;}

public:
    pthread_mutexattr_t m_mutex_attr;
};

// 普通的互斥锁
class Mutex : Noncopyable{
public:

    using Lock = ScopedLock<Mutex>;
    // 初始化互斥量
    Mutex();

    Mutex(const Mutex_Attributes& attrs);
    // 销毁互斥量
    ~Mutex();
    // 对互斥量加锁
    void lock();
    // 尝试对互斥量加锁 true 成功 false 失败
    bool trylock();
    // 对互斥量解锁
    void unlock();

    // 超时锁 默认以秒为单位 
    bool timed_lock(int sec = 1, int msec = 0);

    pthread_mutex_t* native_handle(){ return &m_mutex;}
    const pthread_mutex_t* native_handle() const{ return &m_mutex;}

private:
    pthread_mutex_t m_mutex;
};


// 递归锁
class recursive_mutex {
    using Lock = ScopedLock<recursive_mutex>;

    recursive_mutex();
    ~recursive_mutex();

    void lock();
    void unlock();
    bool trylock();
    
private:
    pthread_mutex_t     m_mutex;

// 如果没有这个变量, 将使用条件变量等,创建递归锁
#ifndef PTHREAD_MUTEX_RECURSIVE
    pthread_cond_t      m_cond;
    bool                m_is_locked;
    pthread_t           m_owner;
    uint32_t            m_count;  // 操作系统默认的原子变量 
#endif
};




}