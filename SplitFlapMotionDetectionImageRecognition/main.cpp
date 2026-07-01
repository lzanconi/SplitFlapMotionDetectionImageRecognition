#include <iostream>
#include "LiveFeedManager.h"	
#include "VideoFeedManager.h"
#include "MotionDetector.h"
#include "ImageTracker.h"
#include "App.h"

int main()
{
	
	App app;

	std::vector<std::string> referenceImages = 
	{
		"SplitFlap.jpg",
		"Whale.jpg"
	};

	//INIT VIDEO FEED
	if (!app.InitVideoFeed("split_flap2.mp4", referenceImages))
	{
		std::cerr << "Error: Could not initialize video feed." << std::endl;
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