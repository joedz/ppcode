#include "../src/config/config.h"
#include "../src/log.h"

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

void test_more_type() {

    


}



int main() { test_config(); }
