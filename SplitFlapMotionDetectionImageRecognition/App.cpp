#include "App.h"
#include "LiveFeedManager.h"
#include "VideoFeedManager.h"	
#include "utils.h"
#include <chrono>

App::App(IFeedManager& feedManager, MotionDetector* motionDetector, ImageTracker* imageTracker)
	: feedManager(feedManager), motionDetector(motionDetector), imageTracker(imageTracker), isRunning(false)
{ }

App::~App()
{
	isRunning = false;
	if (motionDetectThread.joinable())
		motionDetectThread.join();

	if (imageTrackThread.joinable())
		imageTrackThread.join();

	cv::destroyAllWindows();
}

void App::Run()
{
	cv::Mat frame;	
	const std::string mainWindowName = "Split Flap Monitor System";
	const std::string debugWindowName = "Debug Mask";

	isRunning = true;

	// Start long-lived processing threads exactly once
	if (motionDetector != nullptr) 
	{
		motionDetectThread = std::thread(&App::MotionDetectWorkerLoop, this);
	}
	
	if (imageTracker != nullptr) 
	{
		imageTrackThread = std::thread(&App::ImageTrackWorkerLoop, this);
	}


	while (isRunning)
	{
		auto frameStart = std::chrono::steady_clock::now();

		if (!feedManager.ReadNextFrame(frame))
		{
			std::cerr << "Error: Could not read frame from feed." << std::endl;
			break;
		}

		// PRODUCER STEP: Feed the background workers the latest frame.
		// If they are busy working, we overwrite the previous item so they always grab the freshest frame.
		if (motionDetector != nullptr)
		{
			std::lock_guard<std::mutex> lock(motionDetectMutex);
			// Downscaling specifically for worker processing cuts math load by 4x
			cv::resize(frame, motionDetectFrame, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
		}

		if (imageTracker != nullptr)
		{
			std::lock_guard<std::mutex> lock(imageTrackMutex);
			cv::resize(frame, imageTrackFrame, cv::Size(), 0.5, 0.5, cv::INTER_LINEAR);
		}

		// CONSUMER STEP: The main GUI thread reads the results asynchronously.
		// It never blocks or pauses for the computer vision algorithms!
		if (motionDetector != nullptr)
		{
			cv::putText(frame, "Status: " + MotionStateToStr(motionDetector->GetCurrentState()), cv::Point(25, 40),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, motionDetector->GetTextColor(), 2);

			cv::putText(frame, "Motion Pixels: " + std::to_string(motionDetector->GetMotionPixels()), cv::Point(25, 80),
				cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
		}

		if (imageTracker != nullptr && imageTracker->IsTracking())
		{
			cv::putText(frame, "OBJECT TRACKED!", cv::Point(30, 130),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
		}

		

		if (motionDetector != nullptr && motionDetector->ShouldShowDebugMask())
		{
			cv::imshow(debugWindowName, motionDetector->GetThresholdMask());
		}
		else
		{
			double windowProperty = cv::getWindowProperty(debugWindowName, cv::WND_PROP_VISIBLE);
			if (windowProperty > 0)
			{
				cv::destroyWindow(debugWindowName);
			}
		}

		cv::putText(frame, "FPS: " + std::to_string(static_cast<int>(fps)), cv::Point(20, 110),
			cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 0), 1);

		// Display windows smoothly at original video recording speeds
		cv::imshow(mainWindowName, frame);

		HandleKeyboardInput();
		
		Force60FPS(frameStart);
	}

	isRunning = false;
}

void App::Force60FPS(const std::chrono::steady_clock::time_point& frameStart)
{
	// 1. Calculate and update rolling FPS display variable once per second
	frameCount++;
	auto currentTime = std::chrono::steady_clock::now();
	std::chrono::duration<double> elapsed = currentTime - lastTime;
	if (elapsed.count() >= 1.0)
	{
		fps = frameCount / elapsed.count();
		frameCount = 0;
		lastTime = currentTime;
	}

	// 2. Define the exact duration 1 full frame should take at 60 FPS (~16.666 ms)
	const std::chrono::duration<double> frameDuration(1.0 / 60.0);

	// 3. Spin lock until the total duration since frameStart matches our frame time target
	while (std::chrono::steady_clock::now() - frameStart < frameDuration)
	{
		// Relinquish remaining slice of CPU scheduling quantum to prevent 100% core usage spikes
		std::this_thread::yield();
	}
}

void App::HandleKeyboardInput()
{
	char key = (char)cv::waitKey(1);
	if (key == 27) // ESC key
	{
		isRunning = false;
	}
	else if (key == 'd' || key == 'D')
	{
		motionDetector->ToggleDebugMaskDisplay();
	}
}

void App::MotionDetectWorkerLoop()
{
	while (isRunning)
	{
		cv::Mat localFrame;
		{
			// Safely extract the latest shared frame copy
			std::lock_guard<std::mutex> lock(motionDetectMutex);
			if (!motionDetectFrame.empty())
			{
				localFrame = motionDetectFrame.clone();
				motionDetectFrame.release(); // Clear it so we don't re-process the same frame
			}
		}

		if (!localFrame.empty() && motionDetector != nullptr)
		{
			motionDetector->ProcessFrame(localFrame);
		}
		else
		{
			// Give the CPU a tiny rest if no new frames have arrived yet
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
}

void App::ImageTrackWorkerLoop()
{
	while (isRunning)
	{
		cv::Mat localFrame;
		{
			std::lock_guard<std::mutex> lock(imageTrackMutex);
			if (!imageTrackFrame.empty())
			{
				localFrame = imageTrackFrame.clone();
				imageTrackFrame.release();
			}
		}

		if (!localFrame.empty() && imageTracker != nullptr)
		{
			imageTracker->DetectAndMatch(localFrame);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
}


