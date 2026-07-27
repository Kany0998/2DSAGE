#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <deque>

enum LogType
{
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};

struct LogEntry
{
	LogType type;
	std::string message;
};

class Logger
{
	public:
		//Bounded history of recent log entries (e.g. for a future in-game log
		//viewer) - capped at MaxMessages so it doesn't grow for the whole
		//process lifetime.
		static constexpr size_t MaxMessages = 500;
		static std::deque<LogEntry> messages;
		static void Log(const std::string& message);
		static void Err(const std::string& message);
};

#endif // !LOGGER_H
