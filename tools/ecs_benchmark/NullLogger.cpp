// No-op Logger for the ECS benchmark. The engine's src/ECS/ECS.cpp is compiled
// unchanged and logs every created and destroyed entity; the real Logger would
// print each line to the console and time the console instead of the ECS.
// The message strings are still formatted at the call sites in ECS.cpp, so the
// engine side keeps that cost (the legacy copy has its log calls removed), which
// makes the churn comparison conservative for EnTT.
#include "Logger/Logger.h"

std::deque<LogEntry> Logger::messages;

void Logger::Log(const std::string&) {}
void Logger::Warn(const std::string&) {}
void Logger::Err(const std::string&) {}
