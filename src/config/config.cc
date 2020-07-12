#include "config.h"

#include <iostream>

#include "../log.h"

namespace ppcode {

static Logger::ptr g_loger = LOG_ROOT();

// 静态函数 加载 配置项
static void S_listYamlNode(
    const std::string& prefix, const YAML::Node& node,
    std::list<std::pair<std::string, YAML::Node>>& output) {
    if (prefix.find_first_not_of("qwertyuiopasdfghjklzxcvbnm_:.0123456789") !=
        std::string::npos) {
        LOG_ERROR(g_loger) << "Config invaild name:"
                           << " : " << prefix;
        return;
    }
    output.push_back(std::make_pair(prefix, node));

    // 如果是map结点 将数据存入list
    if (node.IsMap()) {
        for (auto it = node.begin(); it != node.end(); ++it) {
            if (prefix.empty()) {
                S_listYamlNode(it->first.Scalar(), it->second, output);
            } else {
                S_listYamlNode(prefix + "." + it->first.Scalar(), it->second,
                               output);
            }
        }
    }
}

void Config::LoadFromYaml(const YAML::Node& rootNode) {
    std::list<std::pair<std::string, YAML::Node>> all_node;
    S_listYamlNode("", rootNode, all_node);

    for (auto& it : all_node) {
        std::string key = it.first;
        if (key.empty()) {
            continue;
        }

        std::transform(key.begin(), key.end(), key.begin(), ::tolower);

        ConfigVarBase::ptr var = LookupBase(key);

        if (var) {
            if (it.second.IsScalar()) {
                var->fromString(it.second.Scalar());
            } else {
                std::stringstream ss;
                ss << it.second;
                var->fromString(ss.str());
            }
        }
    }
}

// 查找配置参数 ,返回配置参数的基类
ConfigVarBase::ptr Config::LookupBase(const std::string& name) {
    RWMutexType::ReadLock lock(Config::GetMutex());
    auto it = GetDatas().find(name);
    return it == GetDatas().end() ? nullptr : it->second;
}

// 便利配置模块中所有配置项
void Config::Visit(std::function<void(ConfigVarBase::ptr)> cb) {
    RWMutexType::ReadLock lock(Config::GetMutex());

    ConfigVarMap& m = GetDatas();
    for (auto it = m.begin(); it != m.end(); ++it) {
        cb(it->second);
    }
}
}  // namespace ppcode