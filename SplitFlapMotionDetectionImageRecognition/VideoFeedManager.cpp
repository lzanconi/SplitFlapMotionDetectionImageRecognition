#include "VideoFeedManager.h"
#include "Logger.h"


VideoFeedManager::VideoFeedManager(const std::string& filePath)
	: videoFilePath(filePath)
{ }

VideoFeedManager::~VideoFeedManager()
{
	videoCapture.release();
	cv::destroyAllWindows();
}

bool VideoFeedManager::Initialize()
{
	videoCapture.open(videoFilePath);
	if (!videoCapture.isOpened())
	{
		Logger::LogApp(MessageType::ERRORS, "VideoFeedManager", "Initialize", "Could not open video file: " + videoFilePath);
		return false;
	}

	return true;
}

bool VideoFeedManager::ReadNextFrame(cv::Mat& frame)
{
	videoCapture >> frame;
	
	if (frame.empty())
	{
		LoopVideo(frame);
	}

	return !frame.empty();
}

int VideoFeedManager::GetFrameWidth()
{
	return static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_WIDTH));
}

int VideoFeedManager::GetFrameHeight()
{
	return static_cast<int>(videoCapture.get(cv::CAP_PROP_FRAME_HEIGHT));
}

bool VideoFeedManager::IsFeedOpen()
{
	return videoCapture.isOpened();
}

void VideoFeedManager::LoopVideo(cv::Mat& frame)
{
	Logger::LogApp(MessageType::INFO, "VideoFeedManager", "LoopVideo", "Video Ended. Looping...");
	videoCapture.set(cv::CAP_PROP_POS_FRAMES, 0);
	videoCapture >> frame;
}
