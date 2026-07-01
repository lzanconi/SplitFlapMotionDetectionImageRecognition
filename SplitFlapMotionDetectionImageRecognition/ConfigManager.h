#pragma once
#include <filesystem>	
#include "customtypes.h"

namespace fs = std::filesystem;

class ConfigManager
{
public:
	Config config;

public:
	ConfigManager() = default;
	~ConfigManager() = default;

	void LoadConfig(const fs::path& configPath);
};

