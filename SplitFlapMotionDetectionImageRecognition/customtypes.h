#pragma once
#include <opencv2/opencv.hpp>

enum class MotionState
{
	NotRotating,
	Rotating
};

enum class MessageType
{
	INFO,
	ERRORS
};

struct ImageTarget
{
	std::string imagePath;
	cv::Mat image;
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
};

struct ROI
{
	int x = 0;
	int y = 0;
	int width = 200;
	int height = 200;
};

struct Config
{
	bool isLiveFeed = false;
	int liveFeedWidth = 1280;
	int liveFeedHeight = 720;
	std::string videoFeedFile = "split_flap2.mp4";
	int motionThreshold = 6000;
	int debounceMotionThreshold = 5;
	int debounceStillThreshold = 15;
	int mog2History = 120;
	int mog2Threshold = 32;
	ROI roi;
	std::vector<std::string> imageTargets = { "SplitFlap.jpg", "Whale.jpg" };
};