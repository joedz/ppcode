#pragma once 

#include "../util/noncopyable.h"

#include <pthread.h>
#include <memory>
#include <string>
#include <functional>
#include <atomic>

namespace ppcode {

class Thread : Noncopyable {
public:
    using ptr = std::shared_ptr<Thread>;

    Thread(const std::string& name, std::function<void()> cb);
    ~Thread();


    // TODO线程属性 
    bool join();
    bool detach();

    bool equal(pthread_t oth);

    //获取线程pid
    pid_t getPid() const { return m_tid;}
    //获取线程pthread id
    pthread_t getThreadId() const { return m_threadId;}

    std::string getName() {return m_name;}
    //bool setName();

public:
    // 设置本线程名称
    static void SetThreadName(const std::string& name);
    // 获取本线程名称 
    static const std::string& GetThreadName() ;
    // 获取本线的结构体
    static Thread* GetThis();
    
private:
    static void* run(void* arg);
    //static void StartThread();
private:
 
    std::function<void()> m_cb;
    pid_t m_tid = -1;
    pthread_t m_threadId = 0;
    std::string m_name;
    
    
    //size_t m_stack_size;
    // run  detach join block exit  
    // std::atomic_bool m_isRun;   //是否已经运行
    // std::atomic_bool m_isDetach;//是否已经被分离
    // std::atomic_bool m_isExit;  //是否已经结束
    //pthread_attr_t m_attr;
};

//http://qiniu.joedz.cn/img/线程pid关系.png

}