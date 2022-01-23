#pragma once

#include "configVar.h"
#include "util/singleton.h"

namespace ppcode {

class Config : public Singleton<Config> {
public:
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;
    using RWMutexType = RWMutex;

    Config() = default;
    ~Config() = default;

    /**
     * @brief 获取/创建对应参数名的配置参数
     * 
     * @tparam T 配置类型参数
     * @param name 配置名称
     * @param default_value 配置的默认值
     * @param description 配置描述
     * @return ConfigVar<T>::ptr 
     */
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(
        const std::string& name, const T& default_value,
        const std::string& description = "") {
        RWMutexType::WriteLock lock(GetMutex());

        auto it = GetDatas().find(name);

        if (it != GetDatas().end()) {
            // 在配置项中找到了
            auto tmp = std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
            if (tmp) {
                LOG_INFO(LOG_ROOT()) << "lookup name" << name << "exists";
                return tmp;
            } else {
                LOG_ERROR(LOG_ROOT())
                    << "Lookup name" << name << " exists but type not "
                    << it->second->toString();
                return nullptr;
            }
        }
        // 没有找到
        if (name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") !=
            std::string::npos) {
            LOG_ERROR(LOG_ROOT()) << "Lookup name invaild" << name;
            throw std::invalid_argument(name);
        }

        typename ConfigVar<T>::ptr value = std::make_shared<ConfigVar<T> >(name, default_value, description);
        GetDatas()[name] = std::dynamic_pointer_cast<ConfigVarBase>(value);

        return value;
    }

    /**
     * @brief 查找配置参数
     * 
     * @tparam T 配置类型
     * @param name 配置名称
     * @return ConfigVar<T>::ptr 
     */
    template <class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if (it == GetDatas().end()) {
            LOG_ERROR(LOG_ROOT()) << "can't find the value, name" << name;
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T>>(it->second);
    }

        /**
     * @brief 查找配置参数 
     * @param name 配置的名字
     * @return ConfigVarBase::ptr 配置参数的基类
     */
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    /**
     * @brief 遍历配置模块中所有配置项
     * @param cb 
     */
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

    /**
     * @brief  从yaml中加载配置
     * @param root YAML Node的根结点
     */
    static void LoadFromYaml(const YAML::Node& root);

    // 从文件中加载配置
    // static void loadFromConfDir(const std::string& path, bool force = false);

private:
    
    static ConfigVarMap& GetDatas() {
        static ConfigVarMap s_data;
        return s_data;
    }

    static RWMutexType& GetMutex() {
        static RWMutexType s_mutex;
        return s_mutex;
    }
};

}  // namespace ppcode
