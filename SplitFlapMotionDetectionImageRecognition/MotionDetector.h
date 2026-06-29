#pragma once
#include <opencv2/opencv.hpp>
#include <iostream>
#include <atomic>
#include <mutex>
#include "customtypes.h"
#include "IFeedManager.h"

class MotionDetector
{
private:
	IFeedManager& feedManager;
	std::atomic<MotionState> currentState;

	int motionThreshold;
	int debounceMotionThreshold;
	int debounceStillThreshold;
	int motionFrameCount;
	int stillFramesCount;
	int mog2History;
	int mog2Threshold;
	std::atomic<bool> showMotionDebugMask;

	cv::Scalar textColor;
	cv::Rect roi;
	cv::Ptr<cv::BackgroundSubtractorMOG2> backgroundSubtractor;

	// Protected data outputs
	cv::Mat latestThresholdMask;
	int latestMotionPixels;
	std::mutex dataMutex;

public:
	MotionDetector(IFeedManager& feedManager);
	~MotionDetector();
	void ValidateROI();	

	void Initialize(int motionThreshold = 6000, int debounceMotionThreshold = 5, int debounceStillThreshold = 15, int mog2History = 120, int mog2Threshold = 32, cv::Rect roi = cv::Rect(473, 112, 507, 443));
	void ProcessFrame(cv::Mat& frame);
	void ToggleDebugMask();

	// Thread-safe access layer
	MotionState GetCurrentState() const { return currentState.load(); }
	cv::Scalar GetTextColor() const { return textColor; }
	bool ShouldShowDebugMask() const { return showMotionDebugMask.load(); }

	int GetLatestMotionPixels()
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		return latestMotionPixels;
	}

	void GetLatestThresholdMask(cv::Mat& dest)
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		if (!latestThresholdMask.empty())
		{
			latestThresholdMask.copyTo(dest);
		}
		else
		{
			dest.release();
		}
	}
};