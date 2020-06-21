#pragma once

#include "log/log_appender.h"
#include "log/log_event.h"
#include "log/log_format.h"
#include "log/log_level.h"
#include "log/log_manager.h"
#include "log/log_warp.h"
#include "log/logger.h"
#include "thread/thread.h"
#include "util/util.h"


// 流的方式写入日志
#define LOG_LEVEL(logger, level)                                            \
    if (logger->getLevel() <= level)                                        \
    ppcode::LogWarp(ppcode::LogEvent::ptr(new ppcode::LogEvent(             \
            logger, time(0), __FILE__, __LINE__, ppcode::GetThreadId(), 23, \
            ppcode::Thread::GetThreadName(), level))).getSS()

// debug 日志
#define LOG_DEBUG(logger) LOG_LEVEL(logger, ppcode::LogLevel::Level::DEBUG)

// INFO 日志
#define LOG_INFO(logger) LOG_LEVEL(logger, ppcode::LogLevel::Level::INFO)

// ERROR 日志
#define LOG_ERROR(logger) LOG_LEVEL(logger, ppcode::LogLevel::Level::ERROR)

// WARN 日志
#define LOG_WARN(logger) LOG_LEVEL(logger, ppcode::LogLevel::Level::WARN)

// ERROR 日志
#define LOG_ERROR(logger) LOG_LEVEL(logger, ppcode::LogLevel::Level::ERROR)

// FATAL 日志
#define LOG_FATAL(logger) LOG_LEVEL(logger, ppcode::LogLevel::Level::FATAL)

#define LOG_ROOT() ppcode::LogManager::getInstance()->getRoot()

#define LOG_NAME(name) ppcode::LogManager::getInstance()->getLogger(name)