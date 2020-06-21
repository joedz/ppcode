#include "thread.h"

#include <stdlib.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <string>

#include "../log.h"
#include "../util/util.h"

namespace ppcode {

// 线程指向自身
static thread_local Thread* t_thread = nullptr;
// 线程名称
static thread_local std::string t_Thread_name = "UNKOWN";

static Logger::ptr g_logger = LOG_ROOT();

//********************** static mothod ************************

// 获取本线程
Thread* Thread::GetThis() { return t_thread; }

// 获取本线程名称
const std::string& Thread::GetThreadName() { return t_Thread_name; }

void Thread::SetThreadName(const std::string& name) {
    t_Thread_name = name;
    // pthread_setname_np()
}

void* Thread::run(void* arg) {
    Thread* thread = (Thread*)arg;

    thread->m_isRun.store(true);

    thread = thread;
    t_Thread_name = thread->m_name;
    thread->m_tid = GetThreadId();//syscall(SYS_gettid);

    // 为线程命名
    pthread_setname_np(thread->m_threadId,
                       thread->m_name.substr(0, 15).c_str());
    // 智能指针可能被lambda捕获，存放在function对象中，直接调用，可能会产生引用计数问题
    std::function<void()> cb;
    cb.swap(thread->m_cb);
    thread->m_sem.notify();

    // 外部函数调用点
    cb();

    //线程正常结束后将调用这里
    LOG_INFO(g_logger) << "thread end";
    
    pthread_exit(nullptr);
}

//********************** Thread ctor **************************

// Thread::Thread(const std::string& name, std::function<void()> cb)
//     :m_cb(cb)
//     ,m_name(name)
// {
//     if(name.empty()) {
//         m_name = "UNKOWN";
//     }

//     int rt = pthread_create(&m_threadId,nullptr, &(Thread::run), this);
//     if(rt) {
//         LOG_ERROR(g_logger) << "pthread_create fail, rt=" << rt
//                             << "create Thread =" << name;
//         throw std::logic_error("pthread_create error");
//     }
// }

void Thread::start_thread() {
    if (m_name.empty()) {
        m_name = "UNKOWN";
    }
    int rt = pthread_create(&m_threadId, nullptr, &(Thread::run), this);

    if (rt) {
        LOG_ERROR(g_logger)
            << "pthread_create fail, rt=" << rt << "create Thread =" << m_name;
        throw std::logic_error("pthread_create error");
    }

    m_sem.wait();
    //使线程启动之后主线程才能离开
}

void Thread::start_thread(const Thread_Attributes& attrs) {
    if (m_name.empty()) {
        m_name = "UNKOWN";
    }

    const pthread_attr_t* thr_attr = attrs.native_handle();
    int rt = pthread_create(&m_threadId, thr_attr, &(Thread::run), this);

    if (rt) {
        LOG_ERROR(g_logger)
            << "pthread_create fail, rt=" << rt << "create Thread =" << m_name;
        throw std::logic_error("pthread_create error");
    }

    m_sem.wait();
    //使线程启动之后主线程才能离开
}

Thread::~Thread() {
    detach();
    m_isExit.store(true);
}

bool Thread::join() {
    if (m_isDetach) {
        LOG_DEBUG(g_logger) << "thread join: thread is detached";
        return false;
    }

    if (m_threadId) {
        int rt = pthread_join(m_threadId, nullptr);
        if (rt) {
            LOG_ERROR(g_logger)
                << "pthread_join fail, rt=" << rt << " join thread=" << m_name;
            throw std::logic_error("thread_join error");
        }
        m_threadId = 0;
    }

    return true;
}

bool Thread::detach() {
    if (m_threadId && !m_isDetach) {
        pthread_detach(m_threadId);  // 分离线程
        m_threadId = 0;
        m_isDetach.store(true);
    }
    return true;
}

bool Thread::equal(pthread_t oth) { return pthread_equal(m_threadId, oth); }

}  // namespace ppcode