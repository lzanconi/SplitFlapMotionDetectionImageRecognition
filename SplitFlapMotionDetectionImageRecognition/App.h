#pragma once
#include "IApp.h"
#include "IFeedManager.h"

#include "utils.h"
#include <thread>
#include <atomic>
#include <mutex>

class LiveFeedManager;
class VideoFeedManager;
class MotionDetector;
class ImageTracker;
class ConfigManager;

class App : public IApp
{
private:
	ConfigManager* configMgr;
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
	MotionState lastMotionState = MotionState::NotRotating;
	bool lastTrackingState = true;
	std::string imageTrackingMsg = "";
	std::string monitoringMsg = "";


public:
	App();
	~App();

	bool Initialize();
	bool InitLiveFeed();
	bool InitVideoFeed();

	void Run();
	Config& GetConfig() override;

private:
	void MotionWorkerLoop();
	void TrackerWorkerLoop();
	void Force60FPS(const std::chrono::steady_clock::time_point& frameStart);
	void HandleKeyboardInput();
	void CheckMotionAndTrackingState();
};