#pragma once
#include <mutex>
#include <fstream>
#include "customtypes.h"

class Logger
{
private:
	static std::mutex logMutex;
	static std::string monitoringLogFilePath;
	static std::string appLogFilePath;

public:
	static void LogMonitoring(const std::string& msg);
	static void LogApp(MessageType type, const std::string& className = "", const std::string& methodName = "", const std::string& message = "");
	static void Clear();
	
private:
	static void ClearLogFile(const std::string& logFilePath);
	static void WriteLogFile(const std::string& logFilePath, const std::string& msg);
};

