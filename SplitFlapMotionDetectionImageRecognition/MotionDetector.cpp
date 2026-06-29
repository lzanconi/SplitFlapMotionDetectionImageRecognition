#include "MotionDetector.h"
#include <opencv2/core/utils/logger.hpp>
#include "utils.h"

MotionDetector::MotionDetector(IFeedManager& feedManager)
	: feedManager(feedManager), currentState(MotionState::NotRotating), latestMotionPixels(0), showMotionDebugMask(false)
{
}

MotionDetector::~MotionDetector() = default;

void MotionDetector::Initialize(int motionThreshold, int debounceMotionThreshold, int debounceStillThreshold, int mog2History, int mog2Threshold, cv::Rect roi)
{
	cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
	this->motionThreshold = motionThreshold;
	this->debounceMotionThreshold = debounceMotionThreshold;
	this->debounceStillThreshold = debounceStillThreshold;
	this->mog2History = mog2History;
	this->mog2Threshold = mog2Threshold;
	this->roi = roi;
	motionFrameCount = 0;
	stillFramesCount = 0;
	textColor = cv::Scalar(0, 255, 0);
	backgroundSubtractor = cv::createBackgroundSubtractorMOG2(this->mog2History, this->mog2Threshold, false);
	ValidateROI();
}

void MotionDetector::ToggleDebugMask()
{
	showMotionDebugMask = !showMotionDebugMask;
}

void MotionDetector::ProcessFrame(cv::Mat& frame)
{
	cv::Mat roi_frame = frame(roi);
	cv::Mat tempMask;
	backgroundSubtractor->apply(roi_frame, tempMask, 0.005);

	int tempPixels = cv::countNonZero(tempMask);

	if (tempPixels > motionThreshold)
	{
		motionFrameCount++;
		stillFramesCount = 0;
		if (motionFrameCount >= debounceMotionThreshold)
		{
			currentState = MotionState::Rotating;
			textColor = cv::Scalar(0, 0, 255);
		}
	}
	else
	{
		stillFramesCount++;
		motionFrameCount = 0;
		if (stillFramesCount >= debounceStillThreshold)
		{
			currentState = MotionState::NotRotating;
			textColor = cv::Scalar(0, 255, 0);
		}
	}

	{
		std::lock_guard<std::mutex> lock(dataMutex);
		tempMask.copyTo(latestThresholdMask);
		latestMotionPixels = tempPixels;
	}
}

void MotionDetector::ValidateROI()
{
	if (feedManager.IsFeedOpen())
	{
		int frameWidth = feedManager.GetFrameWidth();
		int frameHeight = feedManager.GetFrameHeight();
		if (roi.x < 0 || roi.width < 0 || (roi.x + roi.width) > frameWidth ||
			roi.y < 0 || roi.height < 0 || (roi.y + roi.height) > frameHeight)
		{
			this->roi = cv::Rect(0, 0, frameWidth, frameHeight);
		}
	}
}