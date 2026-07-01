#pragma once
#include "IFeedManager.h"
#include "MotionDetector.h"
#include "ImageTracker.h"
#include "LiveFeedManager.h"
#include "VideoFeedManager.h"
#include "utils.h"
#include <thread>
#include <atomic>
#include <mutex>

class App
{
private:
	LiveFeedManager* liveFeedManager;
	VideoFeedManager* videoFeedManager;
	IFeedManager* feedManager;
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
	App();
	~App();

	bool InitLiveFeed(int width = 1280, int height = 720, const std::vector<std::string>& imagePaths = {});
	bool InitVideoFeed(const std::string& videoFilePath, const std::vector<std::string>& imagePaths = {});

	void Run();

private:
	void MotionWorkerLoop();
	void TrackerWorkerLoop();
	void Force60FPS(const std::chrono::steady_clock::time_point& frameStart);
	void HandleKeyboardInput();
};