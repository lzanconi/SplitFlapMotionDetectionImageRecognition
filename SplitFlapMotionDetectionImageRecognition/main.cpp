#include <iostream>
#include "LiveFeedManager.h"	
#include "VideoFeedManager.h"
#include "MotionDetector.h"
#include "ImageTracker.h"
#include "App.h"

int main()
{
	/*LiveFeedManager feedManager(1280, 720);
	if (!feedManager.Initialize())
	{
		std::cerr << "Failed to initialize live feed manager." << std::endl;
		return -1;
	}*/

	VideoFeedManager feedManager("split_flap2.mp4");
	if (!feedManager.Initialize())
	{
		std::cerr << "Failed to initialize video feed manager." << std::endl;
		return -1;
	}

	// 1. Initialize MotionDetector completely
	MotionDetector motionDetector(feedManager);
	motionDetector.Initialize();

	// 2. Initialize ImageTracker completely
	ImageTracker imageTracker(feedManager);
	if (!imageTracker.Initialize("SplitFlap.jpg"))
	{
		std::cerr << "Warning: Tracker module could not target image source path." << std::endl;
	}

	// 3. Now it is completely safe to pass them to App
	App app(feedManager, &motionDetector, &imageTracker);
	app.Run();

	return 0;
}