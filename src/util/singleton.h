#pragma once

#include <memory>
namespace ppcode {


    // 单例模式
    template<class T>
    class Singleton {
    public:

        static T* getInstance(){
            static T s_singleton;
            return &s_singleton;
        }

        Singleton() = default;
        Singleton(const Singleton&) = delete;
    private:
    };

    // 单例智能指针
    template<class T>
    class SingletonPtr {
    public:
        using  singletonPtr_t = std::shared_ptr<T>;
        static std::shared_ptr<T> getInstance(){
            static singletonPtr_t s_singletion
                = std::make_shared<T>();
            return s_singletion;
        }

    private:
        SingletonPtr() = default;
        SingletonPtr(const SingletonPtr&) = delete;
        SingletonPtr& operator=(const SingletonPtr&) = delete;
    private:
    };

}



