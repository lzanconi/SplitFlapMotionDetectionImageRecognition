#include "Logger.h"
#include "utils.h"

std::mutex Logger::logMutex;
std::string Logger::monitoringLogFilePath("Monitoring.log");
std::string Logger::appLogFilePath("App.log");

void Logger::LogMonitoring(const std::string& msg)
{
	std::lock_guard<std::mutex> lock(logMutex);

	const std::string RESET = "\033[0m";
	const std::string GREEN = "\033[32m";
	
	std::string timeStamp = GetTimestampStr();

	std::string monitoringMsg = "---------------------------------------------------------------------------------------------------------\n"
		"[" + timeStamp + "]\n";

	monitoringMsg += msg;
	
	std::cout << GREEN << monitoringMsg << RESET;

	WriteLogFile(monitoringLogFilePath, monitoringMsg);
}

void Logger::LogApp(MessageType type, const std::string& className, const std::string& methodName, const std::string& message)
{
	std::lock_guard<std::mutex> lock(logMutex);

	const std::string RESET = "\033[0m";
	const std::string RED = "\033[31m";
	const std::string GREEN = "\033[32m";
	const std::string YELLOW = "\033[33m";
	const std::string BLUE = "\033[34m";
	const std::string MAGENTA = "\033[35m";
	const std::string CYAN = "\033[36m";

	std::string messageTypeStr = (type == MessageType::ERRORS) ? "ERROR" : "MSG";

	std::string timeStamp = GetTimestampStr();

	std::string logMsg = "---------------------------------------------------------------------------------------------------------\n"
		"[" + messageTypeStr + " -> " + timeStamp + " - " + className + "::" + methodName + "()]:\n " + message + "\n";

	std::cerr << CYAN << logMsg << RESET << std::endl;

	WriteLogFile(appLogFilePath, logMsg);
}

void Logger::Clear()
{
	ClearLogFile(monitoringLogFilePath);
	ClearLogFile(appLogFilePath);
}

void Logger::ClearLogFile(const std::string& logFilePath)
{
	std::lock_guard<std::mutex> lock(logMutex);
	std::string msg = "=== Log File Initialized (" + GetTimestampStr() + ") ===\n";

	std::ofstream logFile(logFilePath, std::ios::out | std::ios::trunc);
	if (logFile.is_open())
	{
		logFile << msg;
		logFile.close();
	}
}

void Logger::WriteLogFile(const std::string& logFilePath, const std::string& msg)
{
	std::ofstream logFile(logFilePath, std::ios::app);
	if (logFile.is_open())
	{
		logFile << msg;
		logFile.close();
	}
}
