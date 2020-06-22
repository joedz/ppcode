#pragma once

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <memory>
#include <string>
#include <functional>
#include <map>
#include <unordered_map>

#include "../log.h"
#include "../thread/rwmutex.h"
#include "../util/util.h"
#include "../util/singleton.h"
#include "lexical_cast.h"

namespace ppcode {

class ConfigVarBase {
public:
    using ptr = std::shared_ptr<ConfigVarBase>;

    ConfigVarBase(const std::string &name, const std::string &description)
        : m_name(name), m_description(description) {
        std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
    }

    virtual ~ConfigVarBase() = default;

    void setName(const std::string &name) { m_name = name; }
    const std::string &getName() const { return m_name; }

    void setdescription(const std::string &description) {
        m_description = description;
    }
    const std::string &getDescription() const { return m_description; }

    // 将配置转换成字符串
    virtual std::string toString() = 0;
    // 将配置从字符串初始化成值
    virtual bool fromString(const std::string &) = 0;
    // 返回配置的参数值的类型名称
    virtual std::string getTypeName() const = 0;

protected:
    std::string m_name;
    std::string m_description;
};


// 配置参数模板类
// T 参数类型   FromStr参数具体类型, Tostr从T转换成成str
template <class T, class FromStr = LexicalCast<std::string, T>, class ToStr = LexicalCast<T, std::string> >
class ConfigVar : public ConfigVarBase {
public:
    using RWMutexType = RWMutex;
    using ptr = std::shared_ptr<ConfigVar>;
    //using std::function<void (const T& old_value, const T& new_value)> on_change_cb;

    ConfigVar(const std::string& name, const T& value, const std::string& description = "")
    :ConfigVarBase(name, description),
    m_val(value){
    }
    virtual ~ConfigVar() = default;

     // 将配置转换成字符串
    virtual std::string toString() override {
        try {
            RWMutexType::ReadLock lock(m_mutex);
            return ToStr()(m_val);
        } catch(std::exception& e) {
            LOG_ERROR(LOG_ROOT()) << "ConfigVar::toString exception"
                << e.what() << "convert: " << 
        }
    }
    // 将配置从字符串初始化成值
    virtual bool fromString(const std::string &) override{

    }
    // 返回配置的参数值的类型名称
    virtual std::string getTypeName() override{

    }

    const T getValue();
    void setValue(const T& v);

private:
    RWMutex m_mutex;
    T m_val;
    //std::map<uint64_t, on_change_cb> m_cbs;
};

class Config : public Singleton<Config>{
public:
    using ConfigVarMap = std::unordered_map<std::string, ConfigVarBase::ptr> ;
    using RWMutexType = RWMutex;
    Config() = default;
    ~Config() = default;
    
    template<class T>
    static typename ConfigVar<T>::ptr Lookup(const std::string& name, 
    const T& default_value, const std::string& description = "");

    template<class T>
    static typename ConfigVar<T>::ptr lookup(const std::string& name);

    // 从yaml中加载配置
    static void LoadFromYaml(const YAML::Node& root);

    // 从文件中加载配置
    //static void loadFromConfDir(const std::string& path, bool force = false);
    
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
