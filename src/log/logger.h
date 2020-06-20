#pragma once 

#include "log_appender.h"
#include "log_event.h"
#include "log_format.h"
#include "log_level.h"
#include "../thread/spinlock.h"

#include <string>
#include <list>
#include <memory>

namespace ppcode {

    // 日志器类 
    class Logger : public std::enable_shared_from_this<Logger>{
    public:
        using ptr = std::shared_ptr<Logger>;
        using MutexType = Spinlock;

        Logger(const std::string& name, LogLevel::Level level = LogLevel::Level::DEBUG);
    
        void log(LogLevel::Level level, LogEvent::ptr event);

        void addAppender( Appender::ptr appender);
        void delAppender( Appender::ptr appender);
        void clearAllAppenders();

        void setName(const std::string& value) { m_name = value;}
        std::string  getName() const { return m_name;}

        void setLevel( LogLevel::Level value) { m_level = value;}
        LogLevel::Level getLevel() const { return m_level;}

        //设致 日志主格式化器  并对没用格式化的日志输出地
        void setFormatter( LogFormatter::ptr value);
         //设致通过字符串 日志主格式化器  并对没用格式化的日志输出地
        void setFormatter(const std::string& value);
        //获取日志主格式化器
        LogFormatter::ptr getFormatter() const { return m_formatter;}

        // 设置root日志器
        Logger::ptr getRoot() { return m_root;}
        void setRoot(Logger::ptr logger) { m_root = logger;}
        
    private:
        std::list<Appender::ptr> m_appenders;
        std::string m_name;
        LogLevel::Level m_level;
        LogFormatter::ptr m_formatter;
        Logger::ptr m_root;
        MutexType m_mutex;
    };

}