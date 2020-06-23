#pragma once

#include <yaml-cpp/yaml.h>

#include <boost/lexical_cast.hpp>
#include <map>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace ppcode {

// 类型转换模板类 Origin源模板   Target 目标模板
template <class Origin, class Target>
class LexicalCast {
public:
    Target operator()(const Origin& value) {
        // boost 库中的字符串与整数/浮点数之间的字面值转换函数 资料

        return boost::lexical_cast<Target>(value);
    }
};

template <class T>
class LexicalCast<std::string, std::vector<T>> {
public:
    std::vector<T> operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        typename std::vector<T> vec;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node;
            //递归转换  将YMAL的str类型转换成T类型 存入 vector
            vec.push_back(LexicalCast<std::string, T>()(ss.str()));
        }
        return vec;
    }
};

// 将vector转换成YAML格式存入字符串
template <class T>
class LexicalCast<std::vector<T>, std::string> {
public:
    std::string operator()(const std::vector<T>& value) {
        YAML::Node node(YAML::NodeType::Sequence);

        for (auto& i : value) {
            // 递归转换 将T类型转成string格式存储
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(i)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// TODO

// string --> list<T>
template <class T>
class LexicalCast<std::string, std::list<T>> {
public:
    std::list<T> operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        typename std::list<T> target_list;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            target_list.push_back(LexicalCast<std::string, T>()(ss.str()));
        }

        return target_list;
    }
};

// list<T> --> string
template <class T>
class LexicalCast<std::list<T>, std::string> {
public:
    std::string operator()(const std::list<T>& value) {
        std::stringstream ss;
        YAML::Node node(YAML::NodeType::Sequence);

        for (auto& it : value) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        ss << node;
        return ss.str();
    }
};

// string --> set<T>
template <class T>
class LexicalCast<std::string, std::set<T>> {
public:
    std::set<T> operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        typename std::set<T> target_set;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            target_set.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return target_set;
    }
};
// set<T> --> string
template <class T>
class LexicalCast<std::set<T>, std::string> {
public:
    std::string operator()(const std::set<T>& value) {
        YAML::Node node(YAML::NodeType::Sequence);

        for (auto& it : value) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// unordered_set<T> --> string
template <class T>
class LexicalCast<std::unordered_set<T>, std::string> {
public:
    std::string operator()(const std::unordered_set<T>& value) {
        YAML::Node node(YAML::NodeType::Sequence);

        for (auto& it : value) {
            node.push_back(YAML::Load(LexicalCast<T, std::string>()(it)));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// string --> unordered_set<T>
template <class T>
class LexicalCast<std::string, std::unordered_set<T>> {
public:
    std::unordered_set<T> operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        typename std::unordered_set<T> target_set;
        std::stringstream ss;
        for (size_t i = 0; i < node.size(); ++i) {
            ss.str("");
            ss << node[i];
            target_set.insert(LexicalCast<std::string, T>()(ss.str()));
        }
        return target_set;
    }
};

// map<std::string, T> --> string
template <class T>
class LexicalCast<std::map<std::string, T>, std::string> {
public:
    std::string operator()(const std::map<std::string, T>& value) {
        YAML::Node node(YAML::NodeType::Map);

        for (auto it = value.begin(); it != value.end(); ++it) {
            node[it->first] =
                YAML::Load(LexicalCast<T, std::string>()(it->second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};

// string --> map<std::string, T>
template <class T>
class LexicalCast<std::string, std::map<std::string, T>> {
public:
    std::map<std::string, T> operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        typename std::map<std::string, T> target_map;
        std::stringstream ss;

        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            target_map.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return target_map;
    }
};

// string --> unordered_map<std::string, T>
template <class T>
class LexicalCast<std::unordered_map<std::string, T>, std::string> {
public:
    std::string operator()(const std::unordered_map<std::string, T>& value) {
        YAML::Node node(YAML::NodeType::Map);

        for (auto it = value.begin(); it != value.end(); ++it) {
            node[it->first] =
                YAML::Load(LexicalCast<T, std::string>()(it->second));
        }
        std::stringstream ss;
        ss << node;
        return ss.str();
    }
};
// unordered_map<std::string, T> --> string
template <class T>
class LexicalCast<std::string, std::unordered_map<std::string, T>> {
public:
    std::unordered_map<std::string, T> operator()(const std::string& value) {
        YAML::Node node = YAML::Load(value);
        typename std::unordered_map<std::string, T> target_map;
        std::stringstream ss;

        for (auto it = node.begin(); it != node.end(); ++it) {
            ss.str("");
            ss << it->second;
            target_map.insert(std::make_pair(it->first.Scalar(), LexicalCast<std::string, T>()(ss.str())));
        }
        return target_map;
    }
};

}  // namespace ppcode
