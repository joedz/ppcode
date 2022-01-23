
#include <iostream>
#include <string.h>

#include "../src/log.h"

using namespace ppcode;

const char *getFileName()
{
	return strrchr(__FILE__, '/');
}

void test_event()
{
	ppcode::LogEvent event;
	event << "string";
	std::cout << event.getSS().str() << std::endl;
}

void test_logger()
{

	Logger::ptr logger(new Logger("root"));
	logger->addAppender(std::make_shared<ConsoleAppender>());
	logger->addAppender(std::make_shared<FileAppender>("lognew"));

	LOG_DEBUG(logger) << "hello world";

	LOG_INFO(logger) << "hello world";

	LOG_FATAL(logger) << "hello world";

	LOG_WARN(logger) << "hello world";
}

Logger::ptr root_logger = LOG_ROOT();

Logger::ptr system_logger = LOG_NAME("system");
void test_log()
{
	LOG_WARN(root_logger) << "hello world";
	LOG_INFO(system_logger) << "hello log system";
}

void test_log_init() {
	std::cout << __LINE__ << std::endl;				// 行号
	std::cout << __FILE__ << std::endl;				// 文件名称
	std::cout << __DATE__ << std::endl;				// 日期
	std::cout << time(0)  << std::endl;				// 时间
	std::cout << __RELATIVE_PATH__ << std::endl;	// 文件名称

	// level
	ppcode::LogLevel::Level e = ppcode::LogLevel::Level::ERROR;

	std::cout << ppcode::LogLevel::ToString(e) << std::endl;

	if (ppcode::LogLevel::FromString("error") == e) {
		std::cout << "equal level" << std::endl;
	}
}

void test_log_format() {
	ppcode::LogFormatter::ptr logFormat = std::make_shared<ppcode::LogFormatter>();

	// logFormat->setPattern("%T%T%P%c");
	Logger::ptr logger(new Logger("root"));
	LogEvent::ptr event(new LogEvent(
		logger,
		time(0), 
		__FILE__, 
		__LINE__, 
		334, 
		44, 
		"main", 
		LogLevel::Level::DEBUG));
	event->getSS() << "hello log";
	logger->addAppender(std::make_shared<ConsoleAppender>());
	logger->addAppender(std::make_shared<FileAppender>("logger"));
	logger->setFormatter(logFormat);

	logger->log(ppcode::LogLevel::Level::DEBUG, event);

	{
		event->setLogger(logger);
		LogWarp::ptr lw(new LogWarp(event));
		lw->getSS() << "www hhhh";
	}
}

void test_log_manager() {
	LogEvent::ptr event(new LogEvent(
		nullptr,
		time(0), 
		__FILE__, 
		__LINE__, 
		334, 
		44, 
		"main", 
		LogLevel::Level::DEBUG));
	event->getSS() << "hello log --";

	auto logManager = LogManager::getInstance();

	// root
	auto rootLogger = logManager->getRoot();
	rootLogger->log(LogLevel::Level::DEBUG, event);

	auto fiberLogger = logManager->getLogger("fiber");
	fiberLogger->addAppender(std::make_shared<FileAppender>("logger"));
	fiberLogger->addAppender(std::make_shared<ConsoleAppender>());

	fiberLogger->log(LogLevel::Level::DEBUG, event);




}

int main()
{
	// 日志格式化类型item测试
	test_log_init();

	// // 测试日志格式化器
	test_log_format();

	// 日志器
	test_log_manager();




	auto logManager = LogManager::getInstance();





	return 0;
}
