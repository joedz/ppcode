#include "log_level.h"
#include <string.h>

namespace ppcode {

	std::vector<std::pair<LogLevel::Level, std::string> > LogLevel::StringLevel
		= {
			{LogLevel::Level::ALL, "ALL"},
			{LogLevel::Level::DEBUG, "DEBUG"},
			{LogLevel::Level::INFO, "INFO"},
			{LogLevel::Level::WARN, "WARN"},
			{LogLevel::Level::ERROR, "ERROR"},
			{LogLevel::Level::FATAL, "FATAL"},
			{LogLevel::Level::OFF, "OFF"}
	};

	std::string LogLevel::ToString(const LogLevel::Level level) {
		if (level > LogLevel::Level::ALL || level < LogLevel::Level::OFF) {
			return StringLevel[(int)level].second;
		}
		return "OFF";
	}

	LogLevel::Level LogLevel::FromString(const std::string& str) {
		for (auto& it : StringLevel) {
			if (strcasecmp(it.second.c_str(), str.c_str()) == 0) {
				return it.first;
			}
		}
		return LogLevel::Level::OFF;
	}

	std::ostream& operator<<(std::ostream& out, const LogLevel::Level level){
		out << LogLevel::ToString(level);
		return out;
	}

}