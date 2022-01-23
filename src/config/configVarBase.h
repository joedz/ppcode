#pragma once

#include <string>
#include <algorithm>
#include <memory>

namespace ppcode {
    /**
     * @brief 配置参数模板类
     * 
     */
    class ConfigVarBase {
    public:
        using ptr = std::shared_ptr<ConfigVarBase>;

        /**
         * @param name 参数名称 
         * @param description 参数描述
         */
        ConfigVarBase(const std::string& name, const std::string& description)
            : m_name(name), m_description(description) {
            std::transform(m_name.begin(), m_name.end(), m_name.begin(), ::tolower);
        }

        virtual ~ConfigVarBase() = default;

        void setName(const std::string& name) { m_name = name; }
        const std::string& getName() const { return m_name; }

        void setdescription(const std::string& description) { m_description = description; }
        const std::string& getDescription() const { return m_description; }

        /**
         * @brief 将配置转换成字符串
         * @return std::string 
         */
        virtual std::string toString() = 0;

        /**
         * @brief 将配置从字符串初始化成值
         * @return true 转换成功
         */
        virtual bool fromString(const std::string&) = 0;
        
        /**
         * @brief 配置的参数值的类型名称
         * @return std::string 
         */
        virtual std::string getTypeName() const = 0;

    protected:
        // 参数名称 (小写)
        std::string m_name;
        // 参数描述
        std::string m_description;
    };

}
