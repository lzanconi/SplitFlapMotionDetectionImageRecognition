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

	//Any frame with more changing pixels than this threshold value is treated as active motion
	int motionThreshold;
	//The number of consecutive motion frames required to trigger a "Rotating" state
	int debounceMotionThreshold;
	//The number of consecutive still frames required to trigger a "Not Rotating" state
	int debounceStillThreshold;
	//Tracks how many consecutive frames have shown significant motion
	int motionFrameCount;
	//Tracks how many consecutive frames have shown little to no motion
	int stillFramesCount;
	//How many past frames the algorithm analyzes to build and mantain its background model.
	int mog2History;
	//How radically a pixel current color/brightness must depart from its historical average to be considered as motion
	int mog2Threshold;

	std::atomic<bool> showMotionDebugMask;
	cv::Scalar textColor;
	//Region of interest for motion detection
	cv::Rect roi;
	//MOG2 background subtractor for motion detection
	cv::Ptr<cv::BackgroundSubtractorMOG2> backgroundSubtractor;

	cv::Mat thresholdMask;
	int motionPixels;
	std::mutex dataMutex;

public:
	MotionDetector(IFeedManager& feedManager);
	~MotionDetector();
	void ValidateROI(const cv::Mat& actualFrame);

	void Initialize(int motionThreshold = 6000, int debounceMotionThreshold = 5, int debounceStillThreshold = 15, int mog2History = 120, int mog2Threshold = 32, cv::Rect roi = cv::Rect(473, 112, 507, 443));
	void ProcessFrame(cv::Mat& frame);
	void ToggleDebugMask();
	void CopyThresholdMaskTo(cv::Mat& dest);

	MotionState GetCurrentState() const { return currentState.load(); }
	cv::Scalar GetTextColor() const { return textColor; }
	bool ShouldShowDebugMask() const { return showMotionDebugMask.load(); }
	int GetMotionPixels();
	
};