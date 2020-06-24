#include "logger.h"
#include <yaml-cpp/yaml.h>
#include <iostream>

namespace ppcode {


Logger::Logger(const std::string& name, LogLevel::Level level)
    : m_name(name), m_level(level) {
    m_formatter = std::make_shared<LogFormatter>(default_log_formatter);
    //std::cout << "模式" << default_log_formatter << std::endl;
}

void Logger::log(LogLevel::Level level, LogEvent::ptr event) {
    if (level < m_level) {
        return;
    }
    auto self = shared_from_this();
    //std::cout << "use default appender :" << m_appenders.size() << std::endl;
    if (m_appenders.empty()) {
        m_root->log(level, event);
        return;
    }
    
    for (auto& it : m_appenders) {
       //  std::cout << "use default appender :" << getYamlString() << m_name << m_appenders.size() << std::endl;
        it->log(self, event);
    }
}

// 添加日志器 如果没有日志格式化器 那么就将主日志格式化器赋给日志输出器
void Logger::addAppender(Appender::ptr appender) {
    MutexType::Lock mylock(m_mutex);

    if (!appender->getFormatter()) {
        appender->setFormatter(m_formatter);
    }
    m_appenders.push_back(appender);
}

void Logger::delAppender(Appender::ptr appender) {
    MutexType::Lock mylock(m_mutex);

    for (auto it = m_appenders.begin(); it != m_appenders.end(); ++it) {
        if (*it == appender) {
            m_appenders.erase(it);
            return;
        }
    }
}

void Logger::clearAllAppenders() {
    MutexType::Lock mylock(m_mutex);

    m_appenders.clear();
}

void Logger::setFormatter(LogFormatter::ptr value) {
    MutexType::Lock mylock(m_mutex);

    m_formatter = value;
    for (auto& it : m_appenders) {
        if (it->isHasFormatter()) {
            it->setFormatter(m_formatter);
        }
    }
}

void Logger::setFormatter(const std::string& value) {
    LogFormatter::ptr newFormat(new LogFormatter(value));
    if (newFormat->isError()) {
        return;
    }

    setFormatter(newFormat);
}


YAML::Node Logger::getYamlNode() {
     YAML::Node node(YAML::NodeType::Map);
    MutexType::Lock mylock(m_mutex);

    node["name"] = m_name;
    node["level"] = LogLevel::ToString(m_level);
    node["formatter"] = m_formatter->getPattern();

    YAML::Node appenders(YAML::NodeType::Sequence);

    for(auto& appender : m_appenders) {
        appenders.push_back(appender->getYamlNode());
    }

    node["appenders"] = appenders;
    return node;
}

std::string Logger::getYamlString(){
    std::stringstream ss;
    ss << getYamlNode();
    return ss.str();
}

}  // namespace ppcode