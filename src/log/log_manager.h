#pragma once

#include "logger.h"
#include "../util/singleton.h"
#include "../thread/mutex.h"
#include "../thread/spinlock.h"

#include <map>
#include <string>

namespace ppcode {

    class LogManager : public Singleton<LogManager>{
    public:
        using MutexType = Spinlock;
        LogManager();

        // 获取一个日志器
        Logger::ptr getLogger(const std::string& name);

        // 获得root 日志器
        Logger::ptr getRoot() const { return m_root;}
    private:
        // key:string value:logger 
        std::map<std::string, Logger::ptr> m_map;
        // 主日志器
        Logger::ptr m_root; 

        MutexType m_mutex;
    };

}