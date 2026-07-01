#include "ConfigManager.h"
#include "Logger.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

void ConfigManager::LoadConfig(const fs::path& configPath)
{
	if (!fs::exists(configPath))
	{
		Logger::LogApp(MessageType::ERRORS, "ConfigManager", "LoadConfig", "Could not load config file: " + configPath.string() + "\n. Use defaults");
		return;
	}

	try
	{ 
		std::fstream fstream(configPath);
		json j = json::parse(fstream);

		if (j.contains("liveFeedWidth"))
			config.liveFeedWidth = j["liveFeedWidth"].get<int>();
		if (j.contains("liveFeedHeight"))
			config.liveFeedHeight = j["liveFeedHeight"].get<int>();
		if (j.contains("videoFeedFile"))
			config.videoFeedFile = j["videoFeedFile"].get<std::string>();


	}
	catch (std::exception& ex)
	{
		Logger::LogApp(MessageType::ERRORS, "ConfigManager", "LoadConfig", "Error parsing config file : " + std::string(ex.what()) + "\n Using defaults.");
	}
}
