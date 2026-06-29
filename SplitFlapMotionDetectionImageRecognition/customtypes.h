#pragma once

#pragma once

enum class MotionState
{
	NotRotating,
	Rotating
};

struct ImageTarget
{
	std::string imagePath;
	cv::Mat image;
	std::vector<cv::KeyPoint> keypoints;
	cv::Mat descriptors;
};