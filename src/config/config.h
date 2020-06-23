#pragma once

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "../log.h"
#include "../thread/rwmutex.h"
#include "../util/singleton.h"
#include "../util/util.h"
#include "lexical_cast.h"
#include "log_config.h"

namespace ppcode {

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;

    ConfigVarBase(const std::string& name, const std::string& description)
        : m_name(name), m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }

    virtual ~ConfigVarBase() = default;

    void setName(const std::string& name) { m_name = name; }
    const std::string& getName() const { return m_name; }

    void setdescription(const std::string& description) {
        m_description = description;
    }
    const std::string& getDescription() const { return m_description; }

    // 将配置转换成字符串
    virtual std::string toString() = 0;
    // 将配置从字符串初始化成值
    virtual bool fromString(const std::string&) = 0;
    // 返回配置的参数值的类型名称
    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};

// 配置参数模板类
// T 参数类型   FromStr参数具体类型, Tostr从T转换成成str
template <class T, class FromStr = LexicalCast<std::string, T>,
          class ToStr = LexicalCast<T, std::string>>
class ConfigVar : public ConfigVarBase {
public:
    using RWMutexType = RWMutex;
    using ptr = std::shared_ptr<ConfigVar>;
    using on_change_cb = std::function<void (const T& old_value, const T& new_value)> ;

    ConfigVar(const std::string& name, const T& value,
              const std::string& description = "")
        : ConfigVarBase(name, description), m_val(value) {}

    virtual ~ConfigVar() = default;

    // 将配置转换成字符串
    virtual std::string toString() override {
        try {
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch (std::exception& e) {
            LOG_ERROR(LOG_ROOT())
                << "ConfigVar::toString exception" << e.what()
                << "convert: " << GetTypeName<T>() << " to string"
                << " name=" << m_name;
        }
        return "";
    }

    // 将配置从字符串初始化成值
    virtual bool fromString(const std::string& value) override {
        try {
            setValue(FromStr()(value));
            return true;
        } catch (std::exception& e) {
            LOG_ERROR(LOG_ROOT()) << "ConfigVar::toString exception" << e.what()
                                  << "convert: to string" << GetTypeName<T>()
                                  << " name=" << m_name;
        }
        return false;
    }

    // 返回配置的参数值的类型名称
    virtual std::string getTypeName() const override { return GetTypeName<T>(); }

    T getValue() {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }
    
    const T& getValue() const {
        RWMutexType::ReadLock lock(m_mutex);
        return m_val;
    }

    void setValue(const T& v) {
        {
            RWMutexType::ReadLock lock(m_mutex);
            if (v == m_val) {
                return;
            }
            // TODO
            for(auto& it : m_cbs) {
                it.second(m_val, v);
            }
        }
        RWMutexType::WriteLock lock(m_mutex);
        m_val = v;
    }

    // 添加 变化回调函数 
    uint64_t addListener(on_change_cb cb) {
        static uint64_t s_fun_id = 0;
        RWMutexType::WriteLock lock(m_mutex);

        ++s_fun_id;
        m_cbs[s_fun_id] = cb;
        return s_fun_id;
    }

    // 删除 变化回调函数
    void delListener(uint64_t key) {
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.erase(key);
    }

    // 获取变化回调函数
    on_change_cb getListener(uint64_t key){
        RWMutexType::ReadLock lock(m_mutex);
        auto it = m_cbs.find(key);
        return it == m_cbs.end() ? nullptr : it->second;
    }

    // 清空变化回调函数
    void clearListener(){
        RWMutexType::WriteLock lock(m_mutex);
        m_cbs.clear();
    }
    
private:
    RWMutex m_mutex;
    T m_val;
    std::map<uint64_t, on_change_cb> m_cbs;
};

class Config : public Singleton<Config> {
public:
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr>;
    using RWMutexType = RWMutex;
    Config() = default;
    ~Config() = default;

    // 获取/创建对应参数名的配置参数
    template <class T>
    static typename ConfigVar<T>::ptr Lookup(
        const std::string& name, const T& default_value,
        const std::string& description = ""){
            RWMutexType::WriteLock lock(GetMutex());

            auto it = GetDatas().find(name);
            
            if(it != GetDatas().end()) {
            // 在配置项中找到了
                auto tmp = std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
                if(tmp) {
                    LOG_INFO(LOG_ROOT()) << "lookup name" << name << "exists";
                    return tmp;
                } else {
                    LOG_ERROR(LOG_ROOT()) << "Lookup name" << name 
                    << " exists but type not " << it->second->toString();
                    return nullptr;
                }
            }
            // 没有找到
            if(name.find_first_not_of("abcdefghijklmnopqrstuvwxyz._0123456789") != std::string::npos) {
                LOG_ERROR(LOG_ROOT()) << "Lookup name invaild" << name;
                throw std::invalid_argument(name);
            }

            typename ConfigVar<T>::ptr value( new ConfigVar<T>(name, default_value, description));
            
            GetDatas()[name] = value;
            return value;
        }

    //查找配置参数
    template <class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name) {
        RWMutexType::ReadLock lock(GetMutex());
        auto it = GetDatas().find(name);
        if(it == GetDatas().end()) {
            LOG_ERROR(LOG_ROOT()) << "can't find the value, name" << name;
            return nullptr;
        }
        return std::dynamic_pointer_cast<ConfigVar<T> >(it->second);
    }

    // 从yaml中加载配置
    static void LoadFromYaml(const YAML::Node& root);

    // 从文件中加载配置
    // static void loadFromConfDir(const std::string& path, bool force = false);

    // 查找配置参数 ,返回配置参数的基类
    static ConfigVarBase::ptr LookupBase(const std::string& name);

    // 便利配置模块中所有配置项
    static void Visit(std::function<void(ConfigVarBase::ptr)> cb);

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
