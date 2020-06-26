#include "../src/log.h"
#include "../src/config/config.h"

#include <string>

static ppcode::Logger::ptr g_logger = LOG_NAME("system");

ppcode::ConfigVar<int>::ptr g_var_int =
    ppcode::Config::Lookup<int>("g_vale_int", (int)100, "g val int");

void test_config() {
    std::cout << g_var_int->getValue() << std::endl;

    std::cout << g_var_int->getTypeName() << std::endl;
    std::cout << g_var_int->getName() << std::endl;
    std::cout << g_var_int->getDescription() << std::endl;
    std::cout << g_var_int->toString() << std::endl;

    auto fun_callback = [](const auto& old_value, const auto& new_value) {
        std::cout << old_value << std::endl;
        std::cout << new_value << std::endl;
    };

    g_var_int->addListener(fun_callback);
    g_var_int->setdescription("dddd");
    g_var_int->setName("ddd");
    g_var_int->setValue(1000);

    std::cout << g_var_int->getValue() << std::endl;

    std::cout << g_var_int->getTypeName() << std::endl;
    std::cout << g_var_int->getName() << std::endl;
    std::cout << g_var_int->getDescription() << std::endl;
    std::cout << g_var_int->toString() << std::endl;
}

ppcode::ConfigVar<bool>::ptr g_bool_var_config = ppcode::Config::Lookup("test.bool", (bool)1, "test bool");
ppcode::ConfigVar<int>::ptr g_int_var_config = ppcode::Config::Lookup("test.int", (int)10, "test int");
ppcode::ConfigVar<uint>::ptr g_uint_var_config = ppcode::Config::Lookup("test.uint", (uint)100, "test uint");
ppcode::ConfigVar<double>::ptr g_double_var_config = ppcode::Config::Lookup("test.double", (double)1000.299, "test double");
ppcode::ConfigVar<float>::ptr g_float_var_config = ppcode::Config::Lookup("test.float", (float)100, "test float");
ppcode::ConfigVar<std::string>::ptr g_string_var_config = ppcode::Config::Lookup("test.string",  std::string("True"), "test string");

ppcode::ConfigVar<std::vector<int> >::ptr g_vector_int_config = ppcode::Config::Lookup("test.vector.int", std::vector<int>{1,2,3}, "test.vector.int");
ppcode::ConfigVar<std::list<int> >::ptr   g_list_int_config = ppcode::Config::Lookup("test.list.int", std::list<int>{1,2,3}, "test.list.int");

// set 
ppcode::ConfigVar<std::set<int> >::ptr    g_set_int_config = ppcode::Config::Lookup("test.set.int", std::set<int>{1,2,3}, "test.set.int");

// unordered_set
ppcode::ConfigVar<std::unordered_set<int> >::ptr    g_unordered_set_int_config = 
            ppcode::Config::Lookup("test.unordered_set.int", std::unordered_set<int>{1,2,3}, "test.unordered_set.int");

// map
ppcode::ConfigVar<std::map<std::string, int> >::ptr    g_map_int_config = 
            ppcode::Config::Lookup("test.map.int", std::map<std::string, int>{{"a",2}, {"b",3}, {"c",4}}, "test.map.int");

// unordered_map
ppcode::ConfigVar<std::unordered_map<std::string, int> >::ptr  g_unordered_map_int_config = 
            ppcode::Config::Lookup("test.unordered_map.int", std::unordered_map<std::string, int>{{"a",2}, {"b",3}, {"c",4}}, "test.unordered_map.int");

void test_more_type() {

    std::cout << "-------------------file load begin---------------------" << std::endl;

#define THE_BASE_TYPE( g_var, name)                                                 \
    {                                                                               \
        const auto& value = g_var->getValue();                                      \
        std::cout << "value = " << value << "   name =" << #name << std::endl;      \
    }

    THE_BASE_TYPE(g_bool_var_config, g_bool);
    THE_BASE_TYPE(g_int_var_config, g_int);
    THE_BASE_TYPE(g_uint_var_config, g_uint);
    THE_BASE_TYPE(g_double_var_config, g_double);
    THE_BASE_TYPE(g_float_var_config, g_float);
    THE_BASE_TYPE(g_string_var_config, g_string);
   

#define THE_CONTAINER_BASE(g_var)               \
    {                                           \
         const auto& cta = g_var->getValue();   \
         std::cout << "value=";                 \
         for(const auto&it : cta) {             \
            std::cout << " " << it;             \
         }                                      \
         std::cout << std::endl;                \
    }

   THE_CONTAINER_BASE(g_vector_int_config);
    THE_CONTAINER_BASE(g_list_int_config);
    THE_CONTAINER_BASE(g_set_int_config);
    THE_CONTAINER_BASE(g_unordered_set_int_config);


#define THE_MAP_TYPE(g_var)                                                 \
    {                                                                       \
        const auto map = g_map_int_config->getValue();                      \
        std::cout << "value= ";                                             \
        for(const auto& it : map) {                                         \
            std::cout << "  "<< it.first << ":" << it.second;               \
        }                                                                   \
        std::cout << std::endl;                                             \
    }

    THE_MAP_TYPE(g_map_int_config);
    THE_MAP_TYPE(g_unordered_map_int_config);

}

void test_load_config_file() {

    YAML::Node node;
    std::cout << "--------------------------load file----------------------------" << std::endl;
    try {
        node = YAML::LoadFile("./test_config.yaml");
        
    } catch(std::exception& e) {
        return;
    }
    ppcode::Config::LoadFromYaml(node);


    THE_BASE_TYPE(g_bool_var_config, g_bool);
    THE_BASE_TYPE(g_int_var_config, g_int);
    THE_BASE_TYPE(g_uint_var_config, g_uint);
    THE_BASE_TYPE(g_double_var_config, g_double);
    THE_BASE_TYPE(g_float_var_config, g_float);
    THE_BASE_TYPE(g_string_var_config, g_string);


    THE_CONTAINER_BASE(g_vector_int_config);
    THE_CONTAINER_BASE(g_list_int_config);
    THE_CONTAINER_BASE(g_set_int_config);
    THE_CONTAINER_BASE(g_unordered_set_int_config);

    THE_MAP_TYPE(g_map_int_config);
    THE_MAP_TYPE(g_unordered_map_int_config);

}

ppcode::ConfigVar<std::vector<std::set<int> > >::ptr g_vector_int_var_config = ppcode::Config::Lookup("test.vector.set.int", 
            std::vector<std::set<int> >{ {1, 2},{2,3,4 },{2,3,3}}, "test.vector.set.int");


void test_config_log(){

    // YAML::Node node = YAML::LoadFile("./test_config.yaml");

    // std::stringstream ss;
    // ss << node["testlog"];
    // //ppcode::LogManager().getInstance()->getRoot();

    // ppcode::Logger::ptr l_logger = ppcode::LexicalCast<std::string, ppcode::Logger>
    //         ()(ss.str());

     ppcode::Logger::ptr l_logger =
     ppcode::LogManager::getInstance()->loadLogger("../test_config.yaml", "testlog");

    std::cout << l_logger->getName()<< std::endl;
    std::cout << l_logger->getLevel() << std::endl;
    std::cout << l_logger->getFormatter()->getPattern() <<std::endl;

    std::cout << l_logger->getYamlString() << std::endl;
    
    LOG_DEBUG(l_logger) << "logger is create";
    LOG_DEBUG(g_logger) << "logger is end";

}


int main() { 
    //test_config(); 

   // test_more_type();
   // test_load_config_file();
    test_config_log();
    
}
