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

struct Config
{
	int liveFeedWidth = 1280;
	int liveFeedHeight = 720;
	std::string videoFeedFile = "split_flap2.mp4";
};