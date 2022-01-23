#pragma once

#include <yaml-cpp/yaml.h>

#include <fstream>
#include <memory>
#include <string>

#include "thread/spinlock.h"
#include "log_event.h"
#include "log_format.h"
#include "log_level.h"

namespace ppcode {
class Logger;

/**
 * @brief 日志输出器的接口类
 * LogAppender可以拥有自己的LogFormat。一个日志器，可以对应多个LogAppender。
 * 也就是说写一条日志，可以落到多个输出，并且每个输出的格式都可以不一样。
 * Appender有单独的日志级别,可以自定义不同级别的日志，输出到不同的Appender，
 * 常用于将错误日志统一输出到一个地方。
 * 目前实现了输出到控制台(StdoutLogAppender)、输出到文件(FileLogAppender)
 */
class Appender {
public:
    // 局部锁
    using ptr = std::shared_ptr<Appender>;
    using MutexType = Spinlock;

    Appender();
    virtual ~Appender(){};

    /**
     * @brief 虚函数 日志输出 
     * 
     * @param logger 日志器
     * @param event 日志内容
     */
    virtual void log(std::shared_ptr<Logger> logger, LogEvent::ptr event) {
        if (!event->getLogger()) {
            event->setLogger(logger);
        }
    }
    
    /**
     * @brief 获取YAML模块节点
     * 
     * @return YAML::Node 
     */
    virtual YAML::Node getYamlNode() = 0;

    /**
     * @brief 设置日志格式化器
     */
    void setFormatter(LogFormatter::ptr value);
    void setFormatter(const std::string& value);

    LogFormatter::ptr getFormatter();
    // std::string getYamlString();

    // 设置 输出器的日志级别
    void setLevel(LogLevel::Level value) { m_level = value; }
    LogLevel::Level getLevel() const { return m_level; }

    // appender输出地的日志级别 是否满足输出
    bool isAppender(LogLevel::Level level) { return level >= m_level; }

    // 是否拥有日志格式化器 如果没有使用 Logger的格式化器
    bool isHasFormatter() const { return m_hasFormatter; }

protected:
    // 日志级别
    LogLevel::Level m_level = LogLevel::Level::DEBUG;
    // 日志格式化器
    LogFormatter::ptr m_formatter;
    // 是否有自己的日志格式化器
    bool m_hasFormatter = false;

    MutexType m_mutex;
};

// @brief 日志输出到控制台
class ConsoleAppender : public Appender {
public:
    using ptr = std::shared_ptr<ConsoleAppender>;
    ConsoleAppender() = default;
    virtual ~ConsoleAppender() = default;
    virtual void log(std::shared_ptr<Logger> logger,
                     LogEvent::ptr event) override;
    virtual YAML::Node getYamlNode() override;
};

// @brief 日志输出到文件 同步
class FileAppender : public Appender {
public:
    using ptr = std::shared_ptr<FileAppender>;
    FileAppender(const std::string& file_name);
    const std::string getPath() const { return m_fileName; }

    virtual ~FileAppender();
    virtual void log(std::shared_ptr<Logger> logger,
                     LogEvent::ptr event) override;
    virtual YAML::Node getYamlNode() override;
private:
    std::ofstream m_logFile;    // 文件流
    std::string m_fileName;     // 文件的路径名
};

}  // namespace ppcode