#pragma once

#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "os.h"

namespace ppcode {

    /**
     * @brief 日志等级
     * 
     */
    class LogLevel {
    public:
        /**
         * @brief 日志等级
         * 
         */
        enum class Level : char {
            ALL,    // 打印所有日志类型
            DEBUG,  // 打印debug信息日志
            INFO,   // 打印调试信息日志
            WARN,   // 打印警告日志
            ERROR,  // 打印错误日志 但是不影响系统
            FATAL,  // 打印错误日志 严重错误
            OFF     // 不打印日志
        };
        static std::string ToString(Level);
        static Level FromString(const std::string&);
    };

    // 通过流的方式输出 Level
    std::ostream& operator<<(std::ostream&, const LogLevel::Level);

}  // namespace ppcode
