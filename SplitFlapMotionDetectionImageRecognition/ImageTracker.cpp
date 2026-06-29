#include "ImageTracker.h"
#include <opencv2/features2d.hpp>
#include <opencv2/calib3d.hpp>

ImageTracker::ImageTracker(IFeedManager& feedManager)
	: feedManager(feedManager), isTracking(false)
{
}

ImageTracker::~ImageTracker()
{
	bruteForceMatcher.release();
	orbDetector.release();
}

bool ImageTracker::Initialize(const std::string& referenceImagePath)
{
	if (!LoadReferenceImage(referenceImagePath, referenceImage)) return false;

	orbDetector = cv::ORB::create(300); // Optimized features limit
	bruteForceMatcher = cv::BFMatcher::create(cv::NORM_HAMMING);

	orbDetector->detectAndCompute(referenceImage, cv::noArray(), referenceKeypoints, referenceDescriptors);
	return !referenceKeypoints.empty() && !referenceDescriptors.empty();
}

void ImageTracker::DetectAndMatch(cv::Mat& frame)
{
	cv::Mat grayFrame;
	cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

	std::vector<cv::KeyPoint> frameKeypoints;
	cv::Mat frameDescriptors;
	orbDetector->detectAndCompute(grayFrame, cv::noArray(), frameKeypoints, frameDescriptors);

	bool currentDetection = false;

	if (referenceDescriptors.empty() || frameDescriptors.empty()) return;

	std::vector<std::vector<cv::DMatch>> knnMatches;
	bruteForceMatcher->knnMatch(referenceDescriptors, frameDescriptors, knnMatches, 2);

	std::vector<cv::DMatch> goodMatches;
	for (size_t i = 0; i < knnMatches.size(); i++)
	{
		if (knnMatches[i].size() >= 2 && knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance)
		{
			goodMatches.push_back(knnMatches[i][0]);
		}
	}

	if (goodMatches.size() >= 4)
	{
		std::vector<cv::Point2f> srcPoints, dstPoints;
		for (size_t i = 0; i < goodMatches.size(); i++)
		{
			srcPoints.push_back(referenceKeypoints[goodMatches[i].queryIdx].pt);
			dstPoints.push_back(frameKeypoints[goodMatches[i].trainIdx].pt);
		}

		std::vector<uchar> inliersMask;
		cv::Mat H = cv::findHomography(srcPoints, dstPoints, cv::RANSAC, 5.0, inliersMask);

		int inliersCount = 0;
		for (size_t i = 0; i < inliersMask.size(); i++) if (inliersMask[i]) inliersCount++;

		if (inliersCount > 12 && !H.empty()) currentDetection = true;
	}

	if (currentDetection && !isTracking.load())
	{
		std::cout << "[STATE] Image detected!" << std::endl;
		isTracking.store(true);
	}
	else if (!currentDetection && isTracking.load())
	{
		std::cout << "[STATE] Tracking lost!" << std::endl;
		isTracking.store(false);
	}
}

bool ImageTracker::LoadReferenceImage(const std::string& imagePath, cv::Mat& image)
{
	image = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
	return !image.empty();
}