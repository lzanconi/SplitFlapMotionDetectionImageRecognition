#pragma once
#include "IFeedManager.h"
#include "MotionDetector.h"
#include "ImageTracker.h"
#include "utils.h"
#include <thread>
#include <atomic>
#include <mutex>

class App
{
private:
	IFeedManager& feedManager;
	MotionDetector* motionDetector;
	ImageTracker* imageTracker;

	std::atomic<bool> isRunning;
	std::thread motionThread;
	std::thread trackerThread;

	cv::Mat latestMotionFrame;
	cv::Mat latestTrackerFrame;
	std::mutex motionMutex;
	std::mutex trackerMutex;

public:
	App(IFeedManager& feedManager, MotionDetector* motionDetector = nullptr, ImageTracker* imageTracker = nullptr);
	~App();

	void Run();

private:
	void MotionWorkerLoop();
	void TrackerWorkerLoop();
};