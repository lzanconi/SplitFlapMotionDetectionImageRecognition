#include "App.h"
#include <iostream>

App::App()
	: feedManager(nullptr), isRunning(false)
{ }

App::~App()
{
	isRunning = false;
	if (motionDetectThread.joinable()) motionDetectThread.join();
	if (imageTrackThread.joinable()) imageTrackThread.join();
	cv::destroyAllWindows();
	
	if (feedManager != nullptr)
	{
		delete feedManager;
		feedManager = nullptr;
	}
}

bool App::InitLiveFeed(int width, int height, const std::vector<std::string>& imagePaths)
{
	liveFeedManager = new LiveFeedManager(width, height);
	if (!liveFeedManager->Initialize())
	{
		std::cerr << "Error: Could not initialize live feed manager." << std::endl;
		return false;
	}

	feedManager = liveFeedManager;

	motionDetector = new MotionDetector(*feedManager);
	motionDetector->Initialize();

	imageTracker = new ImageTracker(*feedManager);
	if (!imagePaths.empty() && !imageTracker->Initialize(imagePaths))
	{
		std::cerr << "Error: Could not initialize image tracker with provided image targets." << std::endl;
		return false;
	}

	return true;
}

bool App::InitVideoFeed(const std::string& videoFilePath, const std::vector<std::string>& imagePaths)
{
	videoFeedManager = new VideoFeedManager(videoFilePath);
	if (!videoFeedManager->Initialize())
	{
		std::cerr << "Error: Could not initialize video feed manager." << std::endl;
		return false;
	}

	feedManager = videoFeedManager;

	motionDetector = new MotionDetector(*feedManager);	
	motionDetector->Initialize();

	imageTracker = new ImageTracker(*feedManager);
	if (!imagePaths.empty() && !imageTracker->Initialize(imagePaths))
	{
		std::cerr << "Error: Could not initialize image tracker with provided image targets." << std::endl;
		return false;
	}

	return true;
}

//Method that runs on a dedicated background thread (motionDetectThread) to process motion detection frames
void App::MotionWorkerLoop()
{
	while (isRunning)
	{
		cv::Mat localFrame;
		{
			std::lock_guard<std::mutex> lock(motionDetectMutex);
			if (!motionDetectFrame.empty())
			{
				localFrame = motionDetectFrame;
				motionDetectFrame.release();
			}
		}

		if (isRunning && !localFrame.empty() && motionDetector != nullptr)
		{
			//Detects if there is motion in the current frame and updates the motion state accordingly
			motionDetector->ProcessFrame(localFrame);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
}

//Method that runs on a dedicated background thread (imageTrackThread) to process image tracking frames
void App::TrackerWorkerLoop()
{
	while (isRunning)
	{
		cv::Mat localFrame;
		{
			std::lock_guard<std::mutex> lock(imageTrackMutex);
			if (!imageTrackFrame.empty())
			{
				localFrame = imageTrackFrame;
				imageTrackFrame.release();
			}
		}

		if (isRunning && !localFrame.empty() && imageTracker != nullptr)
		{
			//Tries to match the reference image with the current frame and updates the tracking state accordingly
			imageTracker->DetectAndMatch(localFrame);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
}

void App::Force60FPS(const std::chrono::steady_clock::time_point& frameStart)
{
	frameCount++;
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = currentTime - lastTime;
	if (elapsed.count() >= 1.0)
	{
		fps = frameCount / elapsed.count();
		frameCount = 0;
		lastTime = currentTime;
	}

	const std::chrono::duration<double> frameDuration(1.0 / 60.0);

	while (std::chrono::steady_clock::now() - frameStart < frameDuration)
	{
		// Relinquish remaining slice of CPU scheduling quantum to prevent 100% core usage spikes
		std::this_thread::yield();
	}
}

void App::Run()
{
	cv::Mat frame;
	const std::string mainWindowName = "Main Window";
	const std::string debugWindowName = "Debug Window";

	isRunning = true;

	//Spawns the motion detection thread
	if (motionDetector != nullptr) 
	{
		motionDetectThread = std::thread(&App::MotionWorkerLoop, this);
	}
	
	//Spawns the image tracking thread
	if (imageTracker != nullptr) 
	{
		imageTrackThread = std::thread(&App::TrackerWorkerLoop, this);
	}

	while (isRunning)
	{
		auto frameStart = std::chrono::steady_clock::now();

		//Get next frame from the active feed manager
		if (!feedManager->ReadNextFrame(frame))
		{
			std::cerr << "Error: Could not read frame from active feed manager." << std::endl;
			break;
		}

		if (frame.empty()) 
			continue;

		//Deep-clone the frame to be used by the motion detection thread
		if (motionDetector != nullptr)
		{
			std::lock_guard<std::mutex> lock(motionDetectMutex);
			motionDetectFrame = frame.clone();
		}

		//Deep-clone the frame to be used by the image tracking thread
		if (imageTracker != nullptr)
		{
			std::lock_guard<std::mutex> lock(imageTrackMutex);
			imageTrackFrame = frame.clone();
		}

		//Draw UI text for motion detection
		if (motionDetector != nullptr)
		{
			cv::putText(frame, "Status: " + MotionStateToStr(motionDetector->GetCurrentState()), cv::Point(25, 40),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, motionDetector->GetTextColor(), 2);

			cv::putText(frame, "Motion Pixels: " + std::to_string(motionDetector->GetMotionPixels()), cv::Point(25, 80),
				cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
		}

		//Draw UI text for image tracking
		if (imageTracker != nullptr && imageTracker->IsTracking())
		{
			std::string msg = "Reference image found: " + imageTracker->GetCurrentTargetPath();
			cv::putText(frame, msg, cv::Point(30, 130),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
		}

		
		cv::Mat displayFrame = frame.clone();

		//Draw the FPS counter on the display frame
		cv::putText(displayFrame, "FPS: " + std::to_string(static_cast<int>(fps)), cv::Point(20, 310),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);
		
		//Draw the video feed into the main window
		cv::imshow(mainWindowName, displayFrame);

		//Draw the motion detection debug mask into the debug window if enabled
		if (motionDetector != nullptr && motionDetector->ShouldShowDebugMask())
		{
			cv::Mat debugMask;
			motionDetector->CopyThresholdMaskTo(debugMask);
			if (!debugMask.empty())
			{
				cv::imshow(debugWindowName, debugMask);
			}
		}
		else
		{
			double windowProperty = cv::getWindowProperty(debugWindowName, cv::WND_PROP_VISIBLE);
			if (windowProperty > 0)
			{
				cv::destroyWindow(debugWindowName);
			}
		}

		HandleKeyboardInput();	
		
		Force60FPS(frameStart);
	}

	isRunning = false;
}

void App::HandleKeyboardInput()
{
	char key = (char)cv::waitKey(1);
	if (key == 27)
	{
		isRunning = false;
	}
	else if ((key == 'd' || key == 'D') && motionDetector != nullptr)
	{
		motionDetector->ToggleDebugMask();
	}
}