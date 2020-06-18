#pragma once 

#include "log_event.h"

#include <string>
#include <vector>
#include <memory>

namespace ppcode {

    // 默认的日志解析格式  must be correct
    extern std::string default_log_formatter;

     class LogFormatterItem {
    public:
        using ptr = std::shared_ptr<LogFormatterItem>;
        LogFormatterItem() = default;
        virtual ~LogFormatterItem() = default;
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) = 0;
    };

    class LogFormatter{
    public:
    // 格式化 日志
        using ptr = std::shared_ptr<LogFormatter>;

        LogFormatter(const std::string& pattern = "");

        bool isError() const { return m_error;}
        void setPattern(const std::string& pattern);
        std::string getPattern() const { return m_pattern;}
        void format(std::ostream& ss, LogEvent::ptr event);
        std::string format(LogEvent::ptr event);
    private:
        void parserFormatter(const std::string& pattern);
        void addEventItem(LogFormatterItem::ptr item);
        void cleanEventItem(LogFormatterItem::ptr item);
    private:
        std::string m_pattern;
        bool m_error = false;
        std::vector<LogFormatterItem::ptr> m_items;
    };
}