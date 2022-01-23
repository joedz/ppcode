#include "log_level.h"
#include <string.h>

namespace ppcode {

std::string LogLevel::ToString(const LogLevel::Level level) {
    switch (level)
    {
#define XX(name)    \
    case LogLevel::Level::name: \
        return #name;   \
        break;

    XX(ALL);
    XX(DEBUG);
    XX(INFO);
    XX(WARN);
    XX(ERROR);
    XX(FATAL);
    XX(OFF);
#undef XX
    default:
        break;
    }
    return "OFF";
}

LogLevel::Level LogLevel::FromString(const std::string& str) {
    static std::vector<std::pair<LogLevel::Level, std::string> > StringLevel = 
{
    {LogLevel::Level::ALL, "ALL"},     
	{LogLevel::Level::DEBUG, "DEBUG"},
    {LogLevel::Level::INFO, "INFO"},   
	{LogLevel::Level::WARN, "WARN"},
    {LogLevel::Level::ERROR, "ERROR"}, 
	{LogLevel::Level::FATAL, "FATAL"},
    {LogLevel::Level::OFF, "OFF"}
};
    if(str.empty()) {
        return LogLevel::Level::DEBUG;
    }
    for (auto& it : StringLevel) {
        if (strcasecmp(it.second.c_str(), str.c_str()) == 0) {
            return it.first;
        }
    }
    return LogLevel::Level::DEBUG;
}

std::ostream& operator<<(std::ostream& out, const LogLevel::Level level) {
    out << LogLevel::ToString(level);
    return out;
}

}  // namespace ppcode