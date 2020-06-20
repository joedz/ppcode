#pragma once 

#include "../util/noncopyable.h"
#include "thread_attributes.h"
#include "semaphore.h"

#include <pthread.h>
#include <memory>
#include <string>
#include <functional>
#include <atomic>
#include <iostream>
namespace ppcode {

// 构造函数中对线程初始化
#define __THREAD_INIT_DATA__        \
        m_isRun{0}                  \
        ,m_isDetach{0}              \
        ,m_isExit{0}


class Thread : Noncopyable {
public:
    using ptr = std::shared_ptr<Thread>;

    //Thread(const std::string& name, std::function<void()> cb);
    ~Thread();

    //线程构造函数 特例化版本1 通过const string& 为线程命名
    template<class F, class... Args>
    Thread(const std::string& name, F&& f, Args&&... args)
    :m_name(name)
    ,__THREAD_INIT_DATA__{
         auto fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        m_cb = std::move(fun);
        //m_cb();
        start_thread();
    }
     //线程构造函数 特例化版本2 通过const char* 为线程命名
    template<class F, class... Args>
    Thread(const char* name, F&& f, Args&&... args)
    :m_name(name)
    ,__THREAD_INIT_DATA__{
        auto fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        m_cb = std::move(fun);
        //m_cb();
        start_thread();
    }

    // 线程构造函数 线程特例化版本3 为线程添加属性
    template<class F, class... Args>
    Thread(const Thread_Attributes& attrs, const std::string& name, F&& f, Args&&... args)
    :m_name(name)
    ,__THREAD_INIT_DATA__{
        auto fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        m_cb = std::move(fun);
        //m_cb();
        start_thread(attrs);
    }

     // 线程构造函数 线程特例化版本4 为线程添加属性
    template<class F, class... Args>
    Thread(const Thread_Attributes& attrs, const char* name, F&& f, Args&&... args)
    :m_name(name)
    ,__THREAD_INIT_DATA__{
        auto fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        m_cb = std::move(fun);
        //m_cb();
        start_thread(attrs);
    }

    template<class F, class... Args>
    Thread( F&& f, Args&&... args)
    :__THREAD_INIT_DATA__{
        auto fun = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
        //使用移动语义，这段代码运行在主线程的栈中空间上，当主线程离开而函数线程未开始运行
        // 函数线程将调用一个非法对象
        m_cb = std::move(fun);
        //m_cb();
        start_thread();
    }

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
    //启动线程函数
    void start_thread();
    // 启动线程函数 并为线程设置属性
    void start_thread(const Thread_Attributes& attrs);
    static void* run(void* arg);
    //static void StartThread();
private:

    std::function<void()> m_cb;
    pid_t m_tid = -1;
    pthread_t m_threadId = 0;
    std::string m_name;
    Semaphore m_sem;
    // run  detach join block exit  
    std::atomic_bool m_isRun;   //是否已经运行
    std::atomic_bool m_isDetach;//是否已经被分离
    std::atomic_bool m_isExit;  //是否已经结束
};


//为主线程命名 main


//http://qiniu.joedz.cn/img/线程pid关系.png

}