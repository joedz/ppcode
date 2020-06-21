#pragma once
#include <sys/time.h>

#include <cerrno>
#include <ctime>
#include <memory>

#include "log_level.h"



namespace ppcode {

class Logger;

class LogEvent {
public:
    using ptr = std::shared_ptr<LogEvent>;
    LogEvent() = default;
    LogEvent(std::shared_ptr<Logger> logger, time_t vtime,
             const std::string& fileName, uint32_t line, uint32_t threadId,
             uint32_t fiberId, const std::string& threadName,
             LogLevel::Level level)
        : m_logger(logger),          // 日志器
          m_time(vtime),             //日志时间
          m_fileName(fileName),      //文件名称
          m_line(line),              //行号
          m_threadId(threadId),      //线程id
          m_fiberId(fiberId),        //协程id
          m_threadName(threadName),  //线程名
          m_level(level) {}          //日志级别

    ~LogEvent() = default;

public:
    template <class _Ty>
    std::stringstream& operator<<(_Ty value) {
        m_ss << value;
        return m_ss;
    }

    std::string toString() const {
        std::stringstream ss;
        ss << m_time << " ";
        ss << m_fiberId << " ";
        ss << m_fileName << " ";
        ss << m_line << " ";
        ss << m_threadId << " ";
        ss << m_fileName << " ";
        ss << LogLevel::ToString(m_level);
        return ss.str();
    }

private:
    std::shared_ptr<Logger> m_logger;
    //时间
    time_t m_time;
    //日志的文件名
    std::string m_fileName;
    //行号
    uint32_t m_line;
    //线程号
    uint32_t m_threadId;
    // 协程号
    uint32_t m_fiberId;
    // 线程名称
    std::string m_threadName;
    // 日志级别
    LogLevel::Level m_level;
    // 日志内容流
    std::stringstream m_ss;

public:
    /** set and get  **/

    void setTime(const time_t& value);
    const time_t& getTime() const { return m_time; }

    void setFileName(const std::string& filename) { m_fileName = filename; }
    const std::string& getFileName() const { return m_fileName; }

    void setLine(const uint32_t line) { m_line = line; }
    uint32_t getLine() const { return m_line; }

    void setThreadId(const uint32_t id) { m_threadId = id; }
    uint32_t getThreadId() const { return m_threadId; }

    void setFiberId(const uint32_t id) { m_fiberId = id; }
    uint32_t getFiberId() const { return m_fiberId; }

    void setThreadName(const std::string& name) { m_threadName = name; }
    const std::string& getThreadName() const { return m_threadName; }

    void setLevel(const LogLevel::Level level) { m_level = level; }
    LogLevel::Level getLevel() const { return m_level; }

    void setSS(const std::string& ss) { m_ss << ss; }
    std::stringstream& getSS() { return m_ss; }

    void setLogger(std::shared_ptr<Logger> logger) { m_logger = logger; }
    std::shared_ptr<Logger> getLogger() const { return m_logger; }

    std::string getContent() const { return m_ss.str(); }
};
}  // namespace ppcode