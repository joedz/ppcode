#include "log_event.h"
#include "log_format.h"
#include "logger.h"

#include <sys/time.h>
#include <string.h>
#include <iostream>

namespace ppcode {
    std::string default_log_formatter = 
    "%D{%y-%m-%d %H:%M:%S}%T%t %N fiber=%F%T[%c] [%P]%T%f:%L%T%e %E%T%m%n";

    // 存储时间的最大缓冲区长度
    const static uint32_t s_time_max_buffer_size = 64;

    //格式化时间
    class TimeFormatter : public LogFormatterItem {
    public:
        TimeFormatter(const std::string& pattern = "%y:%m:%d %H-%M-%S") 
        :m_pattern(pattern){
            m_buffer.resize(s_time_max_buffer_size);
        }
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
            localtime_r( &(event->getTime()), &m_tm);
            strftime(&m_buffer[0], s_time_max_buffer_size, &m_pattern[0], &m_tm);
            
            ss << m_buffer.c_str();
            return ss;
        }
    private:
        std::string m_pattern;
        std::string m_buffer;
        struct tm m_tm;
    };
    
    // 文件名
    class FileNameFormatter : public LogFormatterItem {
    public:
        FileNameFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getFileName();
           return ss;
       }
    };

    // 行号
    class LineFormatter : public LogFormatterItem {
    public:
        LineFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getLine();
           return ss;
       }
    };

    // 线程id
    class ThreadIdFormatter : public LogFormatterItem {
    public:
        ThreadIdFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getThreadId();
           return ss;
       }
    };

    //协程id
    class FiberIdFormatter : public LogFormatterItem {
    public:
        FiberIdFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getFiberId();
           return ss;
       }
    };

    //线程名
    class ThreadNameFormatter : public LogFormatterItem {
    public:
        ThreadNameFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getThreadName();
           return ss;
       }
    };

    // 日志级别
    class LevleFormatter : public LogFormatterItem {
    public:
        LevleFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getLevel();
           return ss;
       }
    };

    //日志名
    class LogNameFormatter : public LogFormatterItem {
    public:
        LogNameFormatter(const std::string& str = ""){}
       virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
           ss << event->getLogger()->getName();
           return ss;
       }
    };

    //错误常量 errno 并打出
    class ErrorFormatter : public LogFormatterItem {
    public:
        ErrorFormatter(const std::string& str = ""){}
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
            ss << "errno="<< errno;
           return ss;
       }
    };

    class ErrorStrFormatter : public LogFormatterItem {
    public:
        ErrorStrFormatter(const std::string& str = ""){}
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override{
            ss << "strerr=" << strerror(errno);
           return ss;
       }
    };

    // tab
    class TabFormatter : public LogFormatterItem {
    public:
        TabFormatter(const std::string& str = ""){}
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override {
            ss << "\t";
            return ss;
        }
    };

    // 换行
    class NewLineFormatter : public LogFormatterItem{
    public:
        NewLineFormatter(const std::string& str = ""){}
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override {
            ss << "\n";
            return ss;
        }
    };
    
    class StringFormatter : public LogFormatterItem{
    public:
        StringFormatter(const std::string& str = "") : m_str(str){ }
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override {
            ss << m_str;
            return ss;
        }
    private:
        std::string m_str;
    };

    class MessageFormatter : public LogFormatterItem{
    public:
        MessageFormatter(const std::string& str = ""){ }
        virtual std::ostream& stream(std::ostream& ss, LogEvent::ptr event) override {
            ss << event->getContent();
            return ss;
        }
    };

    static std::vector<LogFormatterItem::ptr> all_formatter;

    

     /**
     * @brief 构造函数
     * @param[in] pattern 格式模板
     * @details 
     */

    LogFormatter::LogFormatter(const std::string& pattern)
    {
        if(!pattern.empty()) {
            setPattern(pattern);
        }
    }

    void LogFormatter::setPattern(const std::string& pattern){
        parserFormatter(pattern);
        
        if(this->isError()) {
            std::cout << "An error occurred in log parsing:" <<pattern <<
             "Change to the default log format" << std::endl;
            //  parserFormatter(default_log_formatter);
            //  m_pattern = default_log_formatter;
            exit(0);
        }
        m_pattern = pattern;
    }
    
    void LogFormatter::parserFormatter(const std::string& pattern){
        if(pattern.empty()) {
            m_error = true;
            return;
        }

        size_t i = 0;
        std::vector<LogFormatterItem::ptr> items;
        for(; i < pattern.size(); ++i) {
           
            if(pattern[i] == '%') {
                ++i;
                size_t j = i + 1;
                switch(pattern[i]){
                case 'c':   // 日志名
                    items.push_back(std::make_shared<LogNameFormatter>());
                    break;
                case 'D':      //时间 time   %D{}

                    if(j < pattern.size() && pattern[j] == '{') {
                        while(j < pattern.size() && pattern[j] != '}') ++j;

                        if(j >= pattern.size() || pattern[j] != '}') {
                            m_error = true;
                            std::cout << "log patrern: May be missing \"}\"" << std::endl;
                            return;
                        }
                        items.push_back(std::make_shared<TimeFormatter>(pattern.substr(i + 2, j -i -  2)));
                        i = j;
                    } 
                    else {
                        items.push_back(std::make_shared<TimeFormatter>());
                    }
                    break;
                case 'e':   // 错误 erron
                    items.push_back(std::make_shared<ErrorFormatter>());
                    break;
                case 'E':   // 错误 erronStr
                    items.push_back(std::make_shared<ErrorStrFormatter>());
                    break;
                case 'F':   //协程号
                    items.push_back(std::make_shared<FiberIdFormatter>());
                    break;
                case 'f':   // 文件名
                    items.push_back(std::make_shared<FileNameFormatter>());
                    break;
                case 'L':   // 行号
                    items.push_back(std::make_shared<LineFormatter>());
                    break;
                case 'm':   // 消息本体
                    items.push_back(std::make_shared<MessageFormatter>());
                    break;
                case 'N':   // 线程名称
                    items.push_back(std::make_shared<ThreadNameFormatter>());
                    break;
                case 'n':   // 换行 /n
                    items.push_back(std::make_shared<NewLineFormatter>());
                    break;
                case 'P':   // 日志级别
                    items.push_back(std::make_shared<LevleFormatter>());
                    break;
                case 'T':   // Tab
                    items.push_back(std::make_shared<TabFormatter>());
                    break;
                case 't':   // 线程id
                    items.push_back(std::make_shared<ThreadIdFormatter>());
                    break; 
                default:    // 错误
                    std::cout << "log patrern: unkown pattern is %" << pattern[i] << std::endl;
                    m_error = true;
                    return;
                }   // end switch 

            }   // end if(pattern[i] == '%')
            else {
                // 字符
                size_t j = i;
                while(j + 1 < pattern.size() && pattern[j + 1] != '%') ++j;
                items.push_back(std::make_shared<StringFormatter>(pattern.substr(i, j - i + 1)));
                i = j;
            }
            //std::cout << pattern[i] << std::endl;
        }// end whils
        m_items = items;
        m_error = false;
    }

    void LogFormatter::addEventItem(LogFormatterItem::ptr item){
        m_items.push_back(item);
    }

    void LogFormatter::cleanEventItem(LogFormatterItem::ptr item){
        m_items.clear();
    }

    std::string LogFormatter::format(LogEvent::ptr event){
        std::stringstream ss;
        format(ss, event);
        return ss.str();
    }
    void LogFormatter::format(std::ostream& ss, LogEvent::ptr event) {
        for(auto& it : m_items){
            it->stream(ss, event);
        }
    }


}











