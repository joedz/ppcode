#pragma once

#include "log_format.h"
#include "log_level.h"
#include "log_event.h"

#include <string>
#include <fstream>
#include <memory>

#include  "../thread/spinlock.h"

namespace ppcode {
	class Logger;

	// @brief 日志输出方式的接口类
	class Appender {
	public:
		// 局部锁
		using ptr = std::shared_ptr<Appender>;
		using MutexType = Spinlock;

		Appender();
		virtual ~Appender(){};
		//@brief 纯虚函数-写日志
		//@param[in] 一行日志string
		virtual void log(std::shared_ptr<Logger> logger, LogEvent::ptr event) = 0;

		void setFormatter(LogFormatter::ptr value);
		LogFormatter::ptr getFormatter();

		void setLevel(LogLevel::Level value) {m_level = value;}
		LogLevel::Level getLevel() const { return m_level;}
		
		bool isAppender(LogLevel::Level level) {
			return level >= m_level;
		}

		bool isHasFormatter() const { return m_hasFormatter;}

	protected:
		// 日志级别
		LogLevel::Level m_level = LogLevel::Level::DEBUG;
		std::stringstream m_ss;
		// 日志格式化器
		LogFormatter::ptr m_formatter;
		// 是否有自己的日志格式化器
		bool m_hasFormatter = false;

		MutexType m_mutex;
	};

	// @brief 日志输出到控制台
	class ConsoleAppender : public Appender {
	public:
		using ptr = std::shared_ptr<ConsoleAppender>;
		ConsoleAppender() = default;
		virtual ~ConsoleAppender() = default;
		virtual void log(std::shared_ptr<Logger> logger, LogEvent::ptr event) override;

	};

	// @brief 日志输出到文件 同步
	class FileAppender : public Appender {
	public:
		using ptr = std::shared_ptr<FileAppender>;
		FileAppender(const std::string& file_name);
		virtual ~FileAppender() = default;
		virtual void log(std::shared_ptr<Logger> logger, LogEvent::ptr event) override;

	private:
		std::ofstream m_logFile;
		std::string m_fileName; // 文件的路径名
	};

	 

}