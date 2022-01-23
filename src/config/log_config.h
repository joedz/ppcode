#pragma once

#include <string>

#include "../log/log_appender.h"
#include "../log/log_event.h"
#include "../log/log_format.h"
#include "../log/log_level.h"
#include "../log/log_manager.h"
#include "../log/log_warp.h"
#include "../log/logger.h"
#include "../thread/thread.h"
#include "../util/util.h"
#include "lexical_cast.h"

namespace ppcode {

// // 模板特例化 将 YAML::string 转换成 日志输出地
// class LexicalCast<std::string, Appender> {
// public:
//     Appender::ptr operator()(const std::string& value) {
//         YAML::Node node = YAML::Load(value);

//         Appender::ptr appender;

//         if(node["type"].as<std::string>() == "fileappender"){
//             std::string path;
//             if(node["path"].IsScalar()) {
//                 path = node["path"].as<std::string>();
//             } else {
//                 std::cout << "fileappender config is error, name is null" <<
//                 std::endl; throw std::logic_error("name is null");
//             }
//             FileAppender::ptr fileappender(new FileAppender(path));
//             appender = std::dynamic_pointer_cast<Appender>(fileappender);
//         } else if(node["type"].as<std::string>() == "consoleappender"){
//             ConsoleAppender::ptr cappender(new ConsoleAppender);
//             appender = std::dynamic_pointer_cast<Appender>(cappender);
//         }

//         LogLevel::Level level =  LogLevel::FromString(
//         node["level"].IsScalar() ? node["level"].as<std::string>() : "");
//         appender->setLevel(level);

//         if(node["formatter"].IsScalar()) {
//             appender->setFormatter(node["formatter"].as<std::string>());
//         }
//         return appender;
//     }
// };


/**
 * @brief 字符串解析成文件日志输出器
 * 
 * @tparam  
 */
template <>
class LexicalCast<std::string, FileAppender> {
public:
    FileAppender::ptr operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);

        FileAppender::ptr appender;
        //std::cout << "appender=" << value << std::endl;
        std::string path;
        if (node["path"].IsScalar()) {
            path = node["path"].as<std::string>();
        } else {
            std::cout << "fileappender config is error, name is null" << std::endl;
            throw std::logic_error("name is null");
        }
        appender.reset(new FileAppender(path));

        LogLevel::Level level = LogLevel::FromString(node["level"].IsScalar() ? node["level"].as<std::string>() : "");
        appender->setLevel(level);

        if (node["formatter"].IsScalar()) {
            appender->setFormatter(node["formatter"].as<std::string>());
        }
        return appender;
    }
};

/**
 * @brief 将文件日志输出器转换为配置字符串
 * 
 * @tparam  
 */
template <>
class LexicalCast<FileAppender, std::string> {
public:
    std::string operator()(const FileAppender::ptr appender) {
        std::stringstream ss;
        ss << appender->getYamlNode();
        return ss.str();
    }
};

/**
 * @brief 字符串解析成控制台日志输出器
 * 
 * @tparam  
 */
template <>
class LexicalCast<std::string, ConsoleAppender> {
public:
    ConsoleAppender::ptr operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        ConsoleAppender::ptr appender(new ConsoleAppender);

        LogLevel::Level level = LogLevel::FromString(node["level"].IsScalar() ? node["level"].as<std::string>() : "");
        appender->setLevel(level);

        if (node["formatter"].IsScalar()) {
            appender->setFormatter(node["formatter"].as<std::string>());
        }
        return appender;
    }
};

/**
 * @brief 将控制台日志输出器转换为配置字符串
 * 
 * @tparam  
 */
template <>
class LexicalCast<ConsoleAppender, std::string> {
public:
    std::string operator()(const ConsoleAppender::ptr appender) {
        std::stringstream ss;
        ss << appender->getYamlNode();
        return ss.str();
    }
};

/**
 * @brief 将 YAML::string 转换成 logger日志器
 * 
 * @tparam  
 */
template <>
class LexicalCast<std::string, Logger> {
public:
    Logger::ptr operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        if (!node["name"].IsScalar()) {
            std::cout << "log config is error; name is null" << std::endl;
            throw std::logic_error("log config name is null");
        }
        std::stringstream ss;
        ss << node["name"];
        Logger::ptr logger = LogManager::getInstance()->getLogger(ss.str());

        LogLevel::Level level = LogLevel::FromString(
            node["level"].IsScalar() ? node["level"].as<std::string>() : "");
        logger->setLevel(level);

        if (node["formatter"].IsScalar()) {
            //std::cout << node["formatter"].as<std::string>() << std::endl;
            logger->setFormatter(node["formatter"].as<std::string>());
        }

        if (node["appenders"].IsSequence()) {
            logger->clearAllAppenders();

            YAML::Node appenders = node["appenders"];
            for (size_t i = 0; i < appenders.size(); ++i) {
                Appender::ptr appender;
                ss.str("");
                ss << appenders[i];
                //std::cout <<appenders[i] << appenders[i]["type"] << std::endl;
                std::string str = appenders[i]["type"].as<std::string>();
                
                if (str == "fileappender") {
                    FileAppender::ptr fAppender = LexicalCast<std::string, FileAppender>()(ss.str());
                    appender = std::dynamic_pointer_cast<Appender>(fAppender);
                } else if (str == "consoleappender") {
                    ConsoleAppender::ptr cAppender = LexicalCast<std::string, ConsoleAppender>()(ss.str());
                    appender = std::dynamic_pointer_cast<Appender>(cAppender);
                }
                logger->addAppender(appender);
            }
        }
        return logger;
    }
};

/**
 * @brief 将Logger 转换成 YAML::string
 * 
 * @tparam  
 */
template <>
class LexicalCast<Logger, std::string> {
public:
    std::string operator()(const Logger::ptr logger) {
        return logger->getYamlString();
    }
};

}  // namespace ppcode
