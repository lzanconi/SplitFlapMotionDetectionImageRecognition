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

bool ImageTracker::Initialize(const std::vector<std::string>& imagePaths)
{
	//Setup an instance of the ORB detector
	orbDetector = cv::ORB::create(); 
	//Setup an instance of the Brute Force Matcher with Hamming distance
	bruteForceMatcher = cv::BFMatcher::create(cv::NORM_HAMMING);

	int loadedSuccessfully = 0;

	for (const std::string& path : imagePaths)
	{
		ImageTarget target;
		target.imagePath = path;

		// Load the reference image in grayscale
		if (!LoadReferenceImage(path, target.image))
		{
			std::cerr << "Error: Could not load reference image: " << path << std::endl;
			continue;
		}

		// Detect keypoints and compute descriptors for this specific target
		orbDetector->detectAndCompute(target.image, cv::noArray(), target.keypoints, target.descriptors);

		if (!target.keypoints.empty() && !target.descriptors.empty())
		{
			referenceTargets.push_back(target);
			loadedSuccessfully++;
			std::cout << "Successfully initialized reference image: " << path << " (" << target.keypoints.size() << " keypoints found)" << std::endl;
		}
		else
		{
			std::cerr << "Warning: No feature points extracted from " << path << std::endl;
		}
	}

	// Return true if at least one target image was loaded correctly
	return loadedSuccessfully > 0;
}

/*
* Runs continously on a dedicated background thread and determines if a the reference image is detected in the current frame
*/
void ImageTracker::DetectAndMatch(cv::Mat& frame)
{
	// Converts the incoming color frame from the feed to a grayscale frame
	cv::Mat grayFrame;
	cv::cvtColor(frame, grayFrame, cv::COLOR_BGR2GRAY);

	// Extract features from the current frame once
	std::vector<cv::KeyPoint> frameKeypoints;
	cv::Mat frameDescriptors;
	orbDetector->detectAndCompute(grayFrame, cv::noArray(), frameKeypoints, frameDescriptors);

	bool currentDetection = false;

	if (frameDescriptors.empty() || referenceTargets.empty())
		return;

	// Loop through each loaded reference image target to check for matches
	for (const auto& target : referenceTargets)
	{
		if (target.descriptors.empty()) continue;

		std::vector<std::vector<cv::DMatch>> knnMatches;
		// Compares features from the current target against the frame features
		bruteForceMatcher->knnMatch(target.descriptors, frameDescriptors, knnMatches, 2);

		// 1. K-Nearest Neighbors (KNN) Feature Matching
		std::vector<cv::DMatch> goodMatches;
		for (size_t i = 0; i < knnMatches.size(); i++)
		{
			if (knnMatches[i].size() >= 2 && knnMatches[i][0].distance < 0.75 * knnMatches[i][1].distance)
			{
				goodMatches.push_back(knnMatches[i][0]);
			}
		}

		// 2. Homography and RANSAC Outlier Removal
		if (goodMatches.size() >= 4)
		{
			std::vector<cv::Point2f> srcPoints, dstPoints;
			for (size_t i = 0; i < goodMatches.size(); i++)
			{
				srcPoints.push_back(target.keypoints[goodMatches[i].queryIdx].pt);
				dstPoints.push_back(frameKeypoints[goodMatches[i].trainIdx].pt);
			}

			std::vector<uchar> inliersMask;
			cv::Mat H = cv::findHomography(srcPoints, dstPoints, cv::RANSAC, 5.0, inliersMask);

			// 3. Object Detection Verification
			int inliersCount = 0;
			for (size_t i = 0; i < inliersMask.size(); i++)
			{
				if (inliersMask[i]) inliersCount++;
			}

			// If the current target matches adequately, flag detection and stop looking at remaining targets
			if (inliersCount > 12 && !H.empty())
			{
				currentDetection = true;
				break;
			}
		}
	}

	// Trigger state change globally when any tracked image is detected or lost
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