#pragma once
#include <opencv2/opencv.hpp>
#include <vector>
#include <atomic>
#include "IFeedManager.h"
#include "customtypes.h"

class ImageTracker
{
private:
	IFeedManager& feedManager;
	//ORB (Oriented FAST and Rotated BRIEF) is a fast and efficient algorithm to find key points (corners/edges) in an image
	cv::Ptr<cv::ORB> orbDetector;
	//Tries to match the descriptors from the reference image with the descriptors from the current frame
	cv::Ptr<cv::BFMatcher> bruteForceMatcher;
	std::atomic<bool> isTracking;

	std::vector<ImageTarget> referenceTargets;

public:
	ImageTracker(IFeedManager& feedManager);
	~ImageTracker();

	bool Initialize(const std::vector<std::string>& imagePaths);
	void DetectAndMatch(cv::Mat& frame);

	bool IsTracking() const { return isTracking.load(); }

private:
	bool LoadReferenceImage(const std::string& imagePath, cv::Mat& image);
};