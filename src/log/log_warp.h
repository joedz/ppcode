#pragma once

#include "log_event.h"
#include "logger.h"

namespace ppcode {
class LogWarp {
public:
    using ptr = std::shared_ptr<LogWarp>;
    // 日志包装器
    LogWarp(LogEvent::ptr event) : m_event(event) {}
    // 日志包装器 析构时输出日志
    ~LogWarp() { m_event->getLogger()->log(m_event->getLevel(), m_event); }

    // 获取输入日志消息
    std::stringstream& getSS() { return m_event->getSS(); }

private:
    LogEvent::ptr m_event;
};

}  // namespace ppcode
