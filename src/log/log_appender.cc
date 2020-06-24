#include "log_appender.h"

#include <fstream>
#include <iostream>

#include "logger.h"

namespace ppcode {

/*******************************************************************
 * 父类 Appender
 *******************************************************************/
Appender::Appender() : m_hasFormatter(false) {}

void Appender::setFormatter(LogFormatter::ptr value) {
    MutexType::Lock mylock(m_mutex);

    m_formatter = value;
    if (m_formatter) {
        m_hasFormatter = true;
    } else {
        m_hasFormatter = false;
    }
}

void Appender::setFormatter(const std::string& value) {

    LogFormatter::ptr format(new LogFormatter(value));

    setFormatter(format);
}

LogFormatter::ptr Appender::getFormatter() {
    MutexType::Lock mylock(m_mutex);
    return m_formatter;
}

//std::string Appender::getYamlString(){
    //MutexType::lock mylock(m_mutex);

    //std::stringstream ss
    //ss << getYamlNode();
    //retrurn ss.str();
//}


/*******************************************************************
 * 输出到控制台  ConsoleAppender类
 *******************************************************************/

void ConsoleAppender::log(Logger::ptr logger, LogEvent::ptr event) {
    if (isAppender(event->getLevel())) {
        MutexType::Lock mylock(m_mutex);
        if(isHasFormatter()) {
            // bug : 错误的使用 getFormatter()->format(std::cout, event); 导致死锁
            m_formatter->format(std::cout, event);
        } else {
            logger->getFormatter()->format(event);
        }
    }
}

YAML::Node ConsoleAppender::getYamlNode(){
     MutexType::Lock mylock(m_mutex);
    YAML::Node node(YAML::NodeType::Map);
    if(isHasFormatter()) 
        node["formatter"] = m_formatter->getPattern();
    node["level"] = LogLevel::ToString(m_level);
    return node;
 }

/*******************************************************************
 * 输出到文件  FileAppender类
 *******************************************************************/

FileAppender::FileAppender(const std::string& file_name)
    : m_fileName(file_name) {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    m_logFile.open(m_fileName, std::ios::app | std::ios::out);
}

void FileAppender::log(Logger::ptr logger, LogEvent::ptr event) {
    if (isAppender(event->getLevel())) {
        MutexType::Lock mylock(m_mutex);
       if(isHasFormatter()) {
            // bug : 错误的使用 getFormatter()->format(std::cout, event); 导致死锁
            m_formatter->format(m_logFile, event);
        } else {
            logger->getFormatter()->format(event);
        }
        // getFormatter()->format(m_logFile, event);
    }
}

YAML::Node FileAppender::getYamlNode(){
     MutexType::Lock mylock(m_mutex);
    YAML::Node node(YAML::NodeType::Map);

    node["path"] = m_fileName;
    if(isHasFormatter()) 
        node["formatter"] = m_formatter->getPattern();
    node["level"] = LogLevel::ToString(m_level);
    return node;
}

// void FileAppender::append(const std::string& line){
//     // mutex
//     if(!m_logFile.is_open()) {
//         //std::cout << "FileAppender: log file is close" << std::endl;
//         m_logFile.open(m_fileName, std::ios::app | std::ios::out);
//     }
//     m_logFile.write(line.c_str(), line.size());
//     m_logFile.flush();
// }

}  // namespace ppcode