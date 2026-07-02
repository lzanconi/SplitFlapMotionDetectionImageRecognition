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

		if (j.contains("isLiveFeed"))
			config.isLiveFeed = j["isLiveFeed"].get<bool>();
		if (j.contains("liveFeedWidth"))
			config.liveFeedWidth = j["liveFeedWidth"].get<int>();
		if (j.contains("liveFeedHeight"))
			config.liveFeedHeight = j["liveFeedHeight"].get<int>();
		if (j.contains("videoFeedFile"))
			config.videoFeedFile = j["videoFeedFile"].get<std::string>();
		
		//ROI
		if (j.contains("roi"))
		{
			auto roi = j["roi"];
			config.roi.x = roi.value("x", config.roi.x);
			config.roi.y = roi.value("y", config.roi.y);
			config.roi.width = roi.value("width", config.roi.width);
			config.roi.height = roi.value("height", config.roi.height);
		}

		//ImageTargets
		if (j.contains("imageTargets") && j["imageTargets"].is_array())
		{
			config.imageTargets = j["imageTargets"].get<std::vector<std::string>>();
		}

	}
	catch (std::exception& ex)
	{
		Logger::LogApp(MessageType::ERRORS, "ConfigManager", "LoadConfig", "Error parsing config file : " + std::string(ex.what()) + "\n Using defaults.");
	}
}
