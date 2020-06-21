#pragma once

#include <memory>
#include <string>
#include <vector>

#include "log_event.h"

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

// 日志格式化器
class LogFormatter {
public:
    // 格式化 日志
    using ptr = std::shared_ptr<LogFormatter>;

    LogFormatter(const std::string& pattern = "");

    // 日志解析是否出现错误
    bool isError() const { return m_error; }
    // 设置日志解析模式
    void setPattern(const std::string& pattern);
    // 获取日志解析模式
    std::string getPattern() const { return m_pattern; }
    // 格式化日志event
    void format(std::ostream& ss, LogEvent::ptr event);
    std::string format(LogEvent::ptr event);

private:
    // 解析日志格式
    void parserFormatter(const std::string& pattern);
    //添加日志输出项
    void addEventItem(LogFormatterItem::ptr item);
    //清空日志输出项
    void cleanEventItem(LogFormatterItem::ptr item);

private:
    std::string m_pattern;
    bool m_error = false;
    std::vector<LogFormatterItem::ptr> m_items;
};
}  // namespace ppcode