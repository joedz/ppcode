#pragma once

#include <memory>
#include <string>
#include <vector>

#include "log_event.h"

namespace ppcode {

#define default_log_formatter "%D{%y-%m-%d %H:%M:%S}%T%t %N fiber=%F%T[%c] [%P]%T%f:%L%T%e %E %m%n"

/**
 * @brief 日志内容类型基类
 * 
 */
class LogFormatterItem {
public:
    using ptr = std::shared_ptr<LogFormatterItem>;

    LogFormatterItem() = default;
    virtual ~LogFormatterItem() = default;
    virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) = 0;
};

/**
 * @brief 日志内容格式化器
 * 
 */
class LogFormatter {
public:
    // 格式化 日志
    using ptr = std::shared_ptr<LogFormatter>;

    LogFormatter(const std::string& pattern = default_log_formatter);

    bool isError() const { return m_error; }

    // 日志解析模式
    void setPattern(const std::string& pattern);
    std::string getPattern() const { return m_pattern; }

    /**
     * @brief 日志格式化输出
     * 
     * @param ss 输出流
     * @param event 日志事件
     */
    void format(std::ostream& ss, LogEvent::ptr event);

    /**
     * @brief 日志格式化输出
     * 
     * @param event 日志事件
     * @return std::string 
     */
    std::string format(LogEvent::ptr event);

protected:
    /**
     * @brief 日志解析
     * 
     * @param pattern 格式化模式
     */
    void parserFormatter(const std::string& pattern);


    // @brief 操作日志格式输出类型
    void addEventItem(LogFormatterItem::ptr item);
    void cleanEventItem(LogFormatterItem::ptr item);

private:
    std::string m_pattern;
    bool m_error = false;
    std::vector<LogFormatterItem::ptr> m_items;
};
}  // namespace ppcode