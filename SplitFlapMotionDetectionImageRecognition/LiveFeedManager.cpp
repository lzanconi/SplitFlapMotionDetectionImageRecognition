#include "LiveFeedManager.h"
#include "Logger.h"

LiveFeedManager::LiveFeedManager(int width, int height)
	: cameraWidth(width), cameraHeight(height)
{ }

LiveFeedManager::~LiveFeedManager()
{ 
	videoCapture.release();
	cv::destroyAllWindows();
}

bool LiveFeedManager::Initialize()
{
	if (!ScanAndSelectCamera())
	{
		Logger::LogApp(MessageType::ERRORS, "LiveFeedManager", "Initialize", "No camera selected or available!");
		return false;
	}

	return videoCapture.isOpened();
}

bool LiveFeedManager::ReadNextFrame(cv::Mat& frame)
{
	videoCapture >> frame;
	return !frame.empty();
}

bool LiveFeedManager::ScanAndSelectCamera()
{
	std::vector<int> availableCameras;
	std::string msg = "========================================\n";
	msg += " SCANNING FOR AVAILABLE VIDEO DEVICES... \n";
	msg += "========================================\n";

	Logger::LogApp(MessageType::INFO, "LiveFeedManager", "ScanAndSelectCamera", msg);

	for (int i = 0; i < 6; i++)
	{
		cv::VideoCapture foundCamera(i, cv::CAP_DSHOW);
		if (foundCamera.isOpened())
		{
			availableCameras.push_back(i);
			msg = "[" + std::to_string(availableCameras.size() - 1) + "] Camera Index " +  std::to_string(i);

			Logger::LogApp(MessageType::INFO, "LiveFeedManager", "ScanAndSelectCamera", msg);
			foundCamera.release();
		}
	}

	if (availableCameras.empty())
	{
		Logger::LogApp(MessageType::ERRORS, "LiveFeedManager", "ScanAndSelectCamera", "No functional cameras detected");
		return false;
	}

	int selection = 0;
	while (true)
	{
		msg = "\nSelect camera choice (0-" + std::to_string(availableCameras.size() - 1) + "): ";
		Logger::LogApp(MessageType::INFO, "LiveFeedManager", "ScanAndSelectCamera", msg);
		std::cin >> selection;

		if (std::cin.fail() || selection < 0 || selection >= static_cast<int>(availableCameras.size()))
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			msg = "Invalid selection. Please try again.";
			Logger::LogApp(MessageType::INFO, "LiveFeedManager", "ScanAndSelectCamera", msg);
		}
		else
		{
			break;
		}
	}

	int chosenIndex = availableCameras[selection];

	videoCapture.open(chosenIndex, cv::CAP_DSHOW);
	if (!videoCapture.isOpened())
	{
		Logger::LogApp(MessageType::ERRORS, "LiveFeedManager", "ScanAndSelectCamera", "Could not open selected camera!");
		return false;
	}

	videoCapture.set(cv::CAP_PROP_BUFFERSIZE, 1);
	videoCapture.set(cv::CAP_PROP_FPS, 60);
	videoCapture.set(cv::CAP_PROP_FRAME_WIDTH, cameraWidth);
	videoCapture.set(cv::CAP_PROP_FRAME_HEIGHT, cameraHeight);

	return true;
}

int LiveFeedManager::GetFrameWidth()
{
	return cameraWidth;
}

int LiveFeedManager::GetFrameHeight()
{
	return cameraHeight;
}

bool LiveFeedManager::IsFeedOpen()
{
	return videoCapture.isOpened();
}