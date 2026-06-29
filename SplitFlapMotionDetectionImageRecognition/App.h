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
	//Thread that runs motion detection in the background
	std::thread motionDetectThread;
	//Thread that runs image tracking in the background
	std::thread imageTrackThread;

	cv::Mat motionDetectFrame;
	cv::Mat imageTrackFrame;
	std::mutex motionDetectMutex;
	std::mutex imageTrackMutex;

	std::chrono::steady_clock::time_point lastTime = std::chrono::steady_clock::now();
	int frameCount = 0;
	double fps = 0.0;

public:
	App(IFeedManager& feedManager, MotionDetector* motionDetector = nullptr, ImageTracker* imageTracker = nullptr);
	~App();

	void Run();

private:
	void MotionWorkerLoop();
	void TrackerWorkerLoop();
	void Force60FPS(const std::chrono::steady_clock::time_point& frameStart);
	void HandleKeyboardInput();
};