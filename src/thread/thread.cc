#include "thread.h"
#include "../log.h"

#include <string>
#include <atomic>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdlib.h>

namespace ppcode {

// 线程指向自身
static thread_local Thread* t_thread = nullptr;
// 线程名称
static thread_local std::string t_Thread_name = "UNKOWN";

static Logger::ptr g_logger = LOG_ROOT();



//********************** static mothod ************************

Thread* Thread::GetThis(){
    return t_thread;
}

const std::string& Thread::GetThreadName() {
    return t_Thread_name;
}

void Thread::SetThreadName(const std::string& name) {
    t_Thread_name = name;
    //pthread_setname_np()
}


void* Thread::run(void *arg) {
    Thread* thread = (Thread*)arg;

    t_thread = thread;
    t_Thread_name = thread->m_name;
    thread->m_tid = syscall(SYS_gettid);
    pthread_setname_np(thread->m_threadId, thread->m_name.substr(0, 15).c_str());

    std::function<void()> cb;
    cb.swap(thread->m_cb);
    
    // TODO semaphore P
    cb();
    LOG_INFO(g_logger) << "thread end";
    
    return 0;
}

//********************** Thread ctor **************************

Thread::Thread(const std::string& name, std::function<void()> cb)
    :m_cb(cb)
    ,m_name(name)
{
    if(name.empty()) {
        m_name = "UNKOWN";
    }
    
    int rt = pthread_create(&m_threadId,nullptr, &(Thread::run), this);
    if(rt) {
        LOG_ERROR(g_logger) << "pthread_create fail, rt=" << rt
                            << "create Thread =" << name;
        throw std::logic_error("pthread_create error");
    }
    //TODO semaphore w
}

Thread::~Thread(){
    if(m_threadId) {
        // 分离线程
       
        pthread_detach(m_threadId);
        m_threadId = 0; 
    }
}


bool Thread::join(){

    if(m_threadId  ) {
        int rt = pthread_join(m_threadId, nullptr);
        if(rt) {
            LOG_ERROR(g_logger) << "pthread_join fail, rt=" << rt
                    << " join thread=" << m_name;
            throw std::logic_error("thread_join error");
        }
        m_threadId = 0;
    }
    return true;
}

bool Thread::detach() {
    if(m_threadId) {
        pthread_detach(m_threadId);
        m_threadId = 0;
    }
    return true;
}

bool Thread::equal(pthread_t oth){
    return pthread_equal(m_threadId, oth);
}













}