#pragma once 

#include <string>
#include <vector>
#include <memory>
#include <sstream>
#include "../os.h"

namespace ppcode {

	// 日志级别类型
	class LogLevel {
	public:
		enum class Level : char {
			ALL,		// 打印所有日志类型
			DEBUG,		// 打印debug信息日志
			INFO,		// 打印调试信息日志
			WARN,		// 打印警告日志 
			ERROR,		// 打印错误日志 但是不影响系统
			FATAL,		// 打印错误日志 严重错误
			OFF			// 不打印日志
		};
		static std::string ToString(Level level);
		static Level FromString(const std::string& str);
	private:
		static std::vector<std::pair<Level, std::string> > StringLevel;
	};
	// 通过流的方式输出 Level
	std::ostream& operator<<(std::ostream& out, const LogLevel::Level level);
}