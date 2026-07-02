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

	//INIT VIDEO FEED
	/*if (!app.InitVideoFeed())
	{
		return -1;
	}*/

	//INIT LIVE FEED
	/*if (!app.InitLiveFeed())
	{
		return -1;
	}*/

	app.Initialize();
	app.Run();

	return 0;
}