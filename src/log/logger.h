#pragma once

#include <list>
#include <memory>
#include <string>
#include <yaml-cpp/yaml.h>

#include "thread/spinlock.h"
#include "log_appender.h"
#include "log_event.h"
#include "log_format.h"
#include "log_level.h"

namespace ppcode {

/**
 * @brief 日志器
 *  包含一个日志格式化器、一个root Logger、N个Appender。
 *  提供日志写入方法。根据日志器的配置格式和内容，将日志写到对应的地方。
 *  如果写入的日志低于设定的级别，将不会输出。
 */
class Logger : public std::enable_shared_from_this<Logger> {
public:

    using ptr = std::shared_ptr<Logger>;
    using MutexType = Spinlock;

    /**
     * @brief Construct
     * @param name 日志器名称
     * @param level 日志器过滤等级
     */
    Logger(const std::string& name,
           LogLevel::Level level = LogLevel::Level::DEBUG);

    /**
     * @brief 输出日志
     * @param level 输出日志等级
     * @param event 输出日志文本
     */
    void log(LogLevel::Level level, LogEvent::ptr event);

    /**
     * @brief 添加日志输出器
     *  默认是主日志器的日志格式化器
     */
    void addAppender(Appender::ptr appender);
    void delAppender(Appender::ptr appender);
    void clearAllAppenders();

    // formatter
    void setFormatter(LogFormatter::ptr value);
    void setFormatter(const std::string& value);
    LogFormatter::ptr getFormatter() const { return m_formatter; }

    // name
    void setName(const std::string& value) { m_name = value; }
    std::string getName() const { return m_name; }

    // level
    void setLevel(LogLevel::Level value) { m_level = value; }
    LogLevel::Level getLevel() const { return m_level; }

    // root Logger
    Logger::ptr getRoot() { return m_root; }
    void setRoot(Logger::ptr logger) { m_root = logger; }

    // yaml
    std::string getYamlString();
    YAML::Node getYamlNode();
private:
    std::list<Appender::ptr> m_appenders;
    std::string m_name;
    // 日志级别 日志如果低于该级别将不会输出
    LogLevel::Level m_level;
    LogFormatter::ptr m_formatter;
    Logger::ptr m_root;
    MutexType m_mutex;
};

}  // namespace ppcode