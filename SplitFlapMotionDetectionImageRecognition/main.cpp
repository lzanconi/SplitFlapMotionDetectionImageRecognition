#include <opencv2/opencv.hpp>
#include <iostream>
#include "LiveFeedManager.h"
#include "VideoFeedManager.h"
#include "MotionDetector.h"
#include "App.h"

int main() 
{
	VideoFeedManager videoFeedManager("split_flap2.mp4");
	if (!videoFeedManager.Initialize())
	{
		std::cerr << "Failed to initialize the video feed manager." << std::endl;
		return -1;
	}

	/*LiveFeedManager liveFeedManager(1280, 720);
	if (!liveFeedManager.Initialize())
	{
		std::cerr << "Failed to initialize the live feed manager." << std::endl;
		return -1;
	}*/

	MotionDetector motionDetector(videoFeedManager);
	motionDetector.Initialize();

	ImageTracker imageTracker(videoFeedManager);
	if (!imageTracker.Initialize("SplitFlap.jpg"))
	{
		std::cerr << "Failed to initialize the image tracker." << std::endl;
		return -1;
	}	

	//App app(videoFeedManager, &motionDetector, &imageTracker);
	App app(videoFeedManager, &motionDetector, &imageTracker);
	app.Run();

    return 0;
}