#include "MotionDetector.h"
#include <opencv2/core/utils/logger.hpp>
#include "utils.h"

MotionDetector::MotionDetector(IFeedManager& feedManager)
	: feedManager(feedManager), currentState(MotionState::NotRotating), motionPixels(0), showMotionDebugMask(false)
{
}

MotionDetector::~MotionDetector() = default;

void MotionDetector::Initialize(int motionThreshold, int debounceMotionThreshold, int debounceStillThreshold, int mog2History, int mog2Threshold, cv::Rect roi)
{
	this->motionThreshold = motionThreshold;
	this->debounceMotionThreshold = debounceMotionThreshold;
	this->debounceStillThreshold = debounceStillThreshold;
	this->mog2History = mog2History;
	this->mog2Threshold = mog2Threshold;
	this->roi = roi;
	motionFrameCount = 0;
	stillFramesCount = 0;
	textColor = cv::Scalar(0, 255, 0);

	// --- MOG2 (MIXTURE OF GAUSSIANS ---
	// Instead of comparing the current frame to just one previous frame, MOG2 analyzes the history of every single pixel over time 
	// to determine if a change is actual motion of just random noise.
	// The parameters for MOG2 are:
	// 1.history -> how many past frames the algorithm analyzes to build and mantain its background model. 
	//				if the video runs at 60fps, 120 means the algorithm evaluates roughly the last 2 seconds of video
	// 2.varThreshold -> determines how radically a pixel current color/brightness must depart from its historical average to be considered as motion
	// 3.detectShadows -> if true, the algorithm will try to detect shadows and mark them as a separate class of motion.
	backgroundSubtractor = cv::createBackgroundSubtractorMOG2(this->mog2History, this->mog2Threshold, false);
}

void MotionDetector::ProcessFrame(cv::Mat& frame)
{
	ValidateROI(frame);

	//Crop the frame to the region of interest
	cv::Mat roi_frame = frame(roi);
	//Black and white binary matrix where white pixels represent areas of motion and black pixels represent areas of no motion 
	cv::Mat tempMask;

	//Feed the cropped frame to the MOG2 background subtractor to get a binary mask of motion areas
	backgroundSubtractor->apply(roi_frame, tempMask, 0.005);

	//Count how many pixels in the mask are white (255)
	int tempPixels = cv::countNonZero(tempMask);

	//MOTION DETECTED
	//If the number of white pixels exceeds the motionThreshold value, motion is detected
	if (tempPixels > motionThreshold)
	{
		//DEBOUNCE LOGIC
		//Increments the motion frame counter since motion was detected
		motionFrameCount++;
		//Reset the still frame counter since motion was detected
		stillFramesCount = 0;

		//If motion has been detected for a number of consecutive frames > debounceMotionThreshold, the state is set to "Rotating"
		if (motionFrameCount >= debounceMotionThreshold)
		{

			if (currentState.load() != MotionState::Rotating)
			{
				//std::cout << "[MotionDetector] Motion detected!" << std::endl;
			}
			currentState = MotionState::Rotating;
			textColor = cv::Scalar(0, 255, 0);
		}
	}
	//NO MOTION DETECTED
	else
	{
		//DEBOUNCE LOGIC
		//Increments the still frame counter since no motion was detected
		stillFramesCount++;
		//Reset the motion frame counter since no motion was detected
		motionFrameCount = 0;

		//If motion has NOT been detected for a number of consecutive frames > debounceStillThreshold, the state is set to "Not Rotating"
		if (stillFramesCount >= debounceStillThreshold)
		{
			if (currentState.load() != MotionState::NotRotating)
			{
				//std::cout << "[MotionDetector] Motion stopped!" << std::endl;
			}
			currentState = MotionState::NotRotating;
			textColor = cv::Scalar(0, 0, 255);
		}
	}

	//Updates the thead-safe threshold mask and motion pixel count for main thread access
	{
		std::lock_guard<std::mutex> lock(dataMutex);
		tempMask.copyTo(thresholdMask);
		motionPixels = tempPixels;
	}
}

void MotionDetector::ValidateROI(const cv::Mat& actualFrame)
{
	if (feedManager.IsFeedOpen())
	{
		// Use the actual matrix columns and rows, NOT the feedManager variables
		int frameWidth = actualFrame.cols;
		int frameHeight = actualFrame.rows;

		// Clamp the starting coordinates so they cannot be negative or out of bounds
		if (roi.x < 0) roi.x = 0;
		if (roi.y < 0) roi.y = 0;
		if (roi.x >= frameWidth) roi.x = 0;
		if (roi.y >= frameHeight) roi.y = 0;

		// Clamp the width and height so they cannot expand beyond the right/bottom edges
		if (roi.width <= 0 || (roi.x + roi.width) > frameWidth)
		{
			roi.width = frameWidth - roi.x;
		}
		if (roi.height <= 0 || (roi.y + roi.height) > frameHeight)
		{
			roi.height = frameHeight - roi.y;
		}
	}
}

void MotionDetector::ToggleDebugMask()
{
	showMotionDebugMask = !showMotionDebugMask;
}

int MotionDetector::GetMotionPixels()
{
	std::lock_guard<std::mutex> lock(dataMutex);

	return motionPixels;
}

void MotionDetector::CopyThresholdMaskTo(cv::Mat& dest)
{
	std::lock_guard<std::mutex> lock(dataMutex);
	if (!thresholdMask.empty())
	{
		thresholdMask.copyTo(dest);
	}
	else
	{
		dest.release();
	}
}