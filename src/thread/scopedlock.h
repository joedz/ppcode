#pragma once 


#include <atomic>
#include "../util/noncopyable.h"

namespace ppcode {

template<class _Lock>
struct ScopedLock : Noncopyable{
    ScopedLock(_Lock& mutex)
    :m_mutex(mutex){
        m_mutex.lock();
        m_isLocked = true;
    }
    ~ScopedLock(){
        if(m_isLocked) {
            m_mutex.unlock();
            m_isLocked = false;
        }
    }

    void lock(){
        if(!m_isLocked) {
            m_mutex.lock();
            m_isLocked = true;
        }
    }

    bool trylock(){
        if(!m_isLocked) {
           m_isLocked = m_mutex.trylock();
        }
        return m_isLocked;
    }

    void unlock(){
        if(m_isLocked){
            m_mutex.unlock();
            m_isLocked = false;
        }
    }

private:
    _Lock& m_mutex;
    bool m_isLocked;
};



template<class _Lock>
struct WriteScopedLock : Noncopyable{
    WriteScopedLock(_Lock& mutex)
    :m_mutex(mutex){
        m_mutex.wrlock();
        m_isLocked = true;
    }
    ~WriteScopedLock(){
        if(m_isLocked) {
            m_mutex.unlock();
            m_isLocked = false;
        }
    }

    void wrlock(){
        if(!m_isLocked) {
            m_mutex.wrlock();
            m_isLocked = true;
        }
    }

    bool try_wrlock(){
        if(!m_isLocked) {
           m_isLocked = m_mutex.try_wrlock();
        }
        return m_isLocked;
    }

    void unlock(){
        if(m_isLocked){
            m_mutex.unlock();
            m_isLocked = false;
        }
    }

private:
    _Lock& m_mutex;
    bool m_isLocked;
};

template<class _Lock>
struct ReadScopedLock : Noncopyable{
    ReadScopedLock(_Lock& mutex)
    :m_mutex(mutex){
        m_mutex.rdlock();
        m_isLocked = true;
    }
    ~ReadScopedLock(){
        if(m_isLocked) {
            m_mutex.unlock();
            m_isLocked = false;
        }
    }

    void rdlock(){
        if(!m_isLocked) {
            m_mutex.rdlock();
            m_isLocked = true;
        }
    }

    bool try_rdlock(){
        if(!m_isLocked) {
           m_isLocked = m_mutex.try_rdlock();
        }
        return m_isLocked;
    }

    void unlock(){
        if(m_isLocked){
            m_mutex.unlock();
            m_isLocked = false;
        }
    }

private:
    _Lock& m_mutex;
    bool m_isLocked;
};



class CASLock : Noncopyable {
public:
    /// 局部锁
    typedef ScopedLock<CASLock> Lock;


    CASLock() {
        m_mutex.clear();
    }


    ~CASLock() {
    }

    void lock() {
        while(std::atomic_flag_test_and_set_explicit(&m_mutex, std::memory_order_acquire));
    }


    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }
private:
    /// 原子状态
    volatile std::atomic_flag m_mutex;
};




}




