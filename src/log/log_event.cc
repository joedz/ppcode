#include "log_event.h"

namespace ppcode {

    LogEvent::LogEvent(
        std::shared_ptr<Logger> logger,
        time_t vtime,const std::string& fileName,
        uint32_t line, uint32_t threadId,
        uint32_t fiberId,const std::string& threadName,
        LogLevel::Level level)
    :m_logger(logger)
    ,m_time(vtime)
    ,m_fileName(fileName)
    ,m_line(line)
    ,m_threadId(threadId)
    ,m_fiberId(fiberId)
    ,m_threadName(threadName)
    ,m_level(level)
    {
    }
    

    



}