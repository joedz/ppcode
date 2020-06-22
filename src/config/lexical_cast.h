#pragma once

#include <boost/lexical_cast.hpp>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <yaml-cpp/yaml.h>

namespace ppcode {

// 类型转换模板类 Origin源模板   Target 目标模板
template <class Origin, class Target>
class LexicalCast {
public:
    Target operator()(const Origin& value) {
        // boost 库中的字符串与整数/浮点数之间的字面值转换函数 资料
        // Boost库完全开发指南低151页
        return boost::lexical_cast<Target>(value);
    }
};

template <class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& value) {
        YAML::Node node(YAML::NodeType::Sequence);

        for(auto& i : value) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

}  // namespace ppcode
