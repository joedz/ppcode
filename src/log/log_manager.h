#pragma once

#include <map>
#include <string>

#include "thread/mutex.h"
#include "thread/spinlock.h"
#include "util/singleton.h"
#include "logger.h"

namespace ppcode {

/**
 * @brief 日志管理器（单例）
 * 管理所有的日志器。
 * 可以通过解析Yaml配置，动态创建或修改日志器相关的内容（日志级别，日志格式，输出落地点等等）。
 */
class LogManager : public Singleton<LogManager> {
public:
    using MutexType = Spinlock;
    LogManager();
    /**
     * @brief 获取日志器配置 
     * @return std::string 
     */
    std::string getYamlString();

    /**
     * @brief 从文件中加载日志器 
     * @param path 文件目录加文件名 
     * @param loggerName 日志器名称 对应日志名 
     * @return Logger::ptr 
     */
    Logger::ptr loadLogger(const std::string& path, const std::string& loggerName);

    /**
     * @brief 获取日志器 
     * @param name 日志器名称 
     * @return Logger::ptr 
     */
    Logger::ptr getLogger(const std::string& name);
    
    /**
     * @brief 获取主日志器 
     * @return Logger::ptr 
     */
    Logger::ptr getRoot() const { return m_rootLogger; }

private:

    std::map<std::string, Logger::ptr> m_map;
    // 主日志器
    Logger::ptr m_rootLogger;

    MutexType m_mutex;
};

}  // namespace ppcode