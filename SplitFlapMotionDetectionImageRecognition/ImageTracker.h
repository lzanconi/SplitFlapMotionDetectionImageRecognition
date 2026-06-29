#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <atomic>
#include "IFeedManager.h"

class ImageTracker
{
private:
	IFeedManager& feedManager;
	cv::Mat referenceImage;
	std::vector<cv::KeyPoint> referenceKeypoints;
	cv::Mat referenceDescriptors;
	cv::Ptr<cv::ORB> orbDetector;
	cv::Ptr<cv::BFMatcher> bruteForceMatcher;
	std::atomic<bool> isTracking;

public:
	ImageTracker(IFeedManager& feedManager);
	~ImageTracker();

	bool Initialize(const std::string& referenceImagePath);
	void DetectAndMatch(cv::Mat& frame);

	bool IsTracking() const { return isTracking.load(); }

private:
	bool LoadReferenceImage(const std::string& imagePath, cv::Mat& image);
};