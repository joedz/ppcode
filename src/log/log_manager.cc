#include "log_manager.h"
#include "../config/log_config.h"

namespace ppcode {

LogManager::LogManager() {
    m_root.reset(new Logger("root"));
    m_root->addAppender(std::make_shared<ConsoleAppender>());

    m_map[m_root->getName()] = m_root;
}

// 获取一个日志器
Logger::ptr LogManager::getLogger(const std::string& name) {
    MutexType::Lock myLock(m_mutex);

    auto it = m_map.find(name);
    if (it != m_map.end()) {
        return it->second;
    }

    Logger::ptr logger(new Logger(name));
    logger->setRoot(m_root);
    m_map[logger->getName()] = logger;
    return logger;
}

std::string LogManager::getYamlString() {
    MutexType::Lock lock(m_mutex);

    YAML::Node node;
    for(auto&i : m_map) {
        node.push_back(YAML::Load(i.second->getYamlString()));
    }

    std::stringstream ss;
    ss << node;
    return ss.str();
}

Logger::ptr LogManager::loadLogger(const std::string& path, const std::string& loggerName){
    YAML::Node node = YAML::LoadFile(path);

    std::stringstream ss;
    ss << node[loggerName];
    //ppcode::LogManager().getInstance()->getRoot();

    ppcode::Logger::ptr l_logger = ppcode::LexicalCast<std::string, ppcode::Logger>
            ()(ss.str());

    return l_logger;
}


}  // namespace ppcode