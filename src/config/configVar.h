#pragma once

#include <yaml-cpp/yaml.h>

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>

#include "configVarBase.h"
#include "thread/rwmutex.h"
#include "util/util.h"
#include "log.h"
#include "lexical_cast.h"

namespace ppcode
{
    /**
     * @brief 配置参数 - 模版类
     * 
     * @tparam T 参数具体类型
     * @tparam FromStr LexicalCast类型 将string转换成T类型
     * @tparam ToStr LexicalCast类型 将T转换成成string
     */
    template <class T, class FromStr = LexicalCast<std::string, T>,
            class ToStr = LexicalCast<T, std::string>>
    class ConfigVar : public ConfigVarBase {
    public:
        using RWMutexType = RWMutex;
        using ptr = std::shared_ptr<ConfigVar>;
        using on_change_cb =
            std::function<void(const T& old_value, const T& new_value)>;

        ConfigVar(const std::string& name, const T& value,
                const std::string& description = "")
            : ConfigVarBase(name, description), m_val(value) {}

        virtual ~ConfigVar() = default;

        /**
         * @brief 将配置转换成字符串
         * @return std::string 
         */
        virtual std::string toString() override {
            try {
                RWMutexType::ReadLock lock(m_mutex);
                return ToStr()(m_val);
            } catch (std::exception& e) {
                LOG_ERROR(LOG_ROOT())
                    << "ConfigVar::toString exception" 
                    << e.what()
                    << "convert: " << GetTypeName<T>() << " to string"
                    << " name=" << m_name;
            }
            return "";
        }

        /**
         * @brief 将配置从字符串初始化成值
         * @return true 转换成功
         */
        virtual bool fromString(const std::string& value) override {
            try {
                setValue(FromStr()(value));
                return true;
            } catch (std::exception& e) {
                LOG_ERROR(LOG_ROOT()) 
                    << "ConfigVar::toString exception" 
                    << e.what()
                    << "convert: to string" << GetTypeName<T>()
                    << " name=" << m_name;
            }
            return false;
        }

        /**
         * @brief 配置的参数值的类型名称
         * @return std::string 
         */
        virtual std::string getTypeName() const override {
            return GetTypeName<T>();
        }

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

                for (auto& it : m_cbs) {
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
        on_change_cb getListener(uint64_t key) {
            RWMutexType::ReadLock lock(m_mutex);
            auto it = m_cbs.find(key);
            return it == m_cbs.end() ? nullptr : it->second;
        }

        // 清空变化回调函数
        void clearListener() {
            RWMutexType::WriteLock lock(m_mutex);
            m_cbs.clear();
        }

    private:
        RWMutexType m_mutex;
        T m_val;
        std::map<uint64_t, on_change_cb> m_cbs;
    };

} // namespace ppcode
