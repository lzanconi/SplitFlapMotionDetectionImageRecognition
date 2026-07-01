#include <iostream>
#include "LiveFeedManager.h"	
#include "VideoFeedManager.h"
#include "MotionDetector.h"
#include "ImageTracker.h"
#include "App.h"
#include "Logger.h"
#include <opencv2/core/utils/logger.hpp>

int main()
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
	App app;
	Logger::Clear();

	std::vector<std::string> referenceImages = 
	{
		"SplitFlap.jpg",
		"Whale.jpg"
	};

	//INIT VIDEO FEED
	if (!app.InitVideoFeed("split_flap2.mp4", referenceImages))
	{
		return -1;
	}

	//INIT LIVE FEED
	/*if (!app.InitLiveFeed(1280, 720, referenceImages))
	{
		std::cerr << "Error: Could not initialize live feed." << std::endl;
		return -1;
	}*/

	app.Run();

	return 0;
}