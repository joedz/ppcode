#include "log_manager.h"
#include "config/log_config.h"

namespace ppcode {

LogManager::LogManager() {
     // 智能指针初始化
    m_rootLogger.reset(new Logger("root"));

    m_rootLogger->addAppender(std::make_shared<ConsoleAppender>());

    m_map[m_rootLogger->getName()] = m_rootLogger;
}

Logger::ptr LogManager::getLogger(const std::string& name) {
    MutexType::Lock myLock(m_mutex);

    auto it = m_map.find(name);
    if (it != m_map.end()) {
        return it->second;
    }

    Logger::ptr logger(new Logger(name));
    logger->setRoot(m_rootLogger);
    m_map[logger->getName()] = logger;
    return logger;
}

std::string LogManager::getYamlString() {
    MutexType::Lock lock(m_mutex);

    YAML::Node node;
    for(auto&logger : m_map) {
        node.push_back(YAML::Load(logger.second->getYamlString()));
    }

    std::stringstream ss;
    ss << node;
    return ss.str();
}

Logger::ptr LogManager::loadLogger(const std::string& path, const std::string& loggerName) {
    
    YAML::Node node = YAML::LoadFile(path);

    std::stringstream ss;
    ss << node[loggerName];

    ppcode::Logger::ptr l_logger = ppcode::LexicalCast<std::string, ppcode::Logger>
            ()(ss.str());

    return l_logger;
}

}  // namespace ppcode