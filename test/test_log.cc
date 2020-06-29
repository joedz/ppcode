
#include <iostream>
#include <string.h>

#include "../src/log.h"

using namespace ppcode;

void test_Log_Level()
{
	ppcode::LogLevel::Level e;
	e = ppcode::LogLevel::Level::ERROR;

	std::cout << ppcode::LogLevel::ToString(e) << std::endl;

	if (ppcode::LogLevel::FromString("error") == e)
	{
		std::cout << "equal level" << std::endl;
	}
}

const char *getFileName()
{
	return strrchr(__FILE__, '/');
}

void test_log_init()
{
	std::cout << __LINE__ << std::endl;
	std::cout << __FILE__ << std::endl;
	std::cout << __DATE__ << std::endl;
	std::cout << __RELATIVE_PATH__ << std::endl;
}

void test_append()
{
	// old_test The interface has been deleted
	// ppcode::Appender::ptr applog(new ppcode::ConsoleAppender);
	// applog->log("日志");

	// applog.reset(new ppcode::FileAppender("test.log"));
	// applog->append("日志");
}

void test_event()
{
	ppcode::LogEvent event;
	event << "string";
	std::cout << event.getSS().str() << std::endl;
}

void test_format()
{

	ppcode::LogFormatter::ptr format1 = std::make_shared<ppcode::LogFormatter>();

	format1->setPattern("%T%T%P%c");
	//std::cout << event->toString() << std::endl;

	Logger::ptr logger(new Logger("root"));
	LogEvent::ptr event(new LogEvent(
		logger,
		time(0), __FILE__, __LINE__, 334, 44, "main", LogLevel::Level::DEBUG));
	event->getSS() << "hello log";
	logger->addAppender(std::make_shared<ConsoleAppender>());
	logger->addAppender(std::make_shared<FileAppender>("lognew"));

	logger->log(LogLevel::Level::ERROR, event);

	{
		event->setLogger(logger);
		LogWarp::ptr lw(new LogWarp(event));
		lw->getSS() << "www hhhh";
	}
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

int main()
{
	test_Log_Level();
	test_log_init();
	test_append();
	test_event();
	test_format();
	test_logger();
	test_log();

	LOG_DEBUG(root_logger) << "massage";
	test_logger();
	return 0;
}
