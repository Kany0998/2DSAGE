#include "Logger.h"
#include <iostream>
#include <ctime>
#include <chrono>
#include <ctime>

std::deque<LogEntry> Logger::messages;


std::string CurrentDateTimeToString()
{
    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::tm tm_buf;
    localtime_s(&tm_buf, &now);

    std::string output(30, '\0');
    std::strftime(&output[0], output.size(), "%d-%b-%Y %H:%M:%S", &tm_buf);
    output.erase(output.find('\0'));
    return output;
}


void Logger::Log(const std::string& message)
{
	LogEntry logEntry;
	logEntry.type = LOG_INFO;
	logEntry.message = "LOG: [" + CurrentDateTimeToString() + "] " + message;
	std::cout << "\x1B[32m" << logEntry.message << "\033[0m" << std::endl;
	messages.push_back(logEntry);
	if (messages.size() > MaxMessages)
	{
		messages.pop_front();
	}
}

void Logger::Err(const std::string& message)
{
	LogEntry logEntry;
	logEntry.type = LOG_ERROR;
	logEntry.message = "ERR: [" + CurrentDateTimeToString() + "] " + message;
	std::cerr << "\x1B[91m" << logEntry.message << "\033[0m" << std::endl;
	messages.push_back(logEntry);
	if (messages.size() > MaxMessages)
	{
		messages.pop_front();
	}
}