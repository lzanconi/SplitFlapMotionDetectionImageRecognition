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
	std::atomic<bool> isRunning = false;
	IFeedManager& feedManager;
	MotionDetector* motionDetector;
	ImageTracker* imageTracker;
	std::thread motionDetectThread;
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
	void Force60FPS(const std::chrono::steady_clock::time_point& frameStart);
	void HandleKeyboardInput();
	void MotionDetectWorkerLoop();
	void ImageTrackWorkerLoop();
};

