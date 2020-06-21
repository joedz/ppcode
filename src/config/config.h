#pragma once

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <memory>
#include <string>

#include "../log.h"
#include "../thread/rwmutex.h"

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

template <class T, class FromStr, class ToStr>
class ConfigVar : public ConfigVarBase {
public:

};

}  // namespace ppcode
