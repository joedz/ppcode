#pragma once

#include <pthread.h>
#include <sys/types.h>


namespace ppcode {

// 线程属性
class Thread_Attributes {
public:
    //对线程属性进行初始化
    Thread_Attributes();

    //释放线程属性
    ~Thread_Attributes();

    //设置线程栈空间大小
    void set_stack_size(size_t size);

    //获取线程属性空间大小
    size_t get_stack_size();

    //获取原始属性结构
    pthread_attr_t* native_handle(){
        return &m_attr;
    }

    // 获取const类型原始属性结构
    const pthread_attr_t* native_handle() const{
        return &m_attr;
    }

private:
    // 线程属性结构体
    pthread_attr_t m_attr;
};

}