#include "App.h"
#include <iostream>

App::App(IFeedManager& feedManager, MotionDetector* motionDetector, ImageTracker* imageTracker)
	: feedManager(feedManager), motionDetector(motionDetector), imageTracker(imageTracker), isRunning(false)
{
	// FIX: Do not spawn threads here anymore! 
	// Spawning them here causes a race condition if modules are not initialized yet.
}

App::~App()
{
	isRunning = false;
	if (motionThread.joinable()) motionThread.join();
	if (trackerThread.joinable()) trackerThread.join();
	cv::destroyAllWindows();
}

void App::MotionWorkerLoop()
{
	while (isRunning)
	{
		cv::Mat localFrame;
		{
			std::lock_guard<std::mutex> lock(motionMutex);
			if (!latestMotionFrame.empty())
			{
				localFrame = latestMotionFrame;
				latestMotionFrame.release();
			}
		}

		if (isRunning && !localFrame.empty() && motionDetector != nullptr)
		{
			motionDetector->ProcessFrame(localFrame);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
}

void App::TrackerWorkerLoop()
{
	while (isRunning)
	{
		cv::Mat localFrame;
		{
			std::lock_guard<std::mutex> lock(trackerMutex);
			if (!latestTrackerFrame.empty())
			{
				localFrame = latestTrackerFrame;
				latestTrackerFrame.release();
			}
		}

		if (isRunning && !localFrame.empty() && imageTracker != nullptr)
		{
			imageTracker->DetectAndMatch(localFrame);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(2));
		}
	}
}

void App::Run()
{
	cv::Mat frame;
	const std::string mainWindowName = "Split Flap Monitor System";
	const std::string debugWindowName = "MOG2 Foreground Mask (Debug)";

	isRunning = true;

	if (motionDetector != nullptr) {
		motionThread = std::thread(&App::MotionWorkerLoop, this);
	}
	if (imageTracker != nullptr) {
		trackerThread = std::thread(&App::TrackerWorkerLoop, this);
	}

	while (isRunning)
	{
		if (!feedManager.ReadNextFrame(frame))
		{
			std::cerr << "Error: Could not read frame from active feed manager." << std::endl;
			break;
		}

		if (frame.empty()) continue;

		// FIX: Send full resolution deep copies to the thread loops.
		// This keeps the original frame bounds intact so the hardcoded ROI doesn't break.
		if (motionDetector != nullptr)
		{
			std::lock_guard<std::mutex> lock(motionMutex);
			latestMotionFrame = frame.clone();
		}

		if (imageTracker != nullptr)
		{
			std::lock_guard<std::mutex> lock(trackerMutex);
			latestTrackerFrame = frame.clone();
		}

		// Draw text notifications on the local main-thread frame
		if (motionDetector != nullptr)
		{
			cv::putText(frame, "Status: " + MotionStateToStr(motionDetector->GetCurrentState()), cv::Point(25, 40),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, motionDetector->GetTextColor(), 2);

			cv::putText(frame, "Motion Pixels: " + std::to_string(motionDetector->GetLatestMotionPixels()), cv::Point(25, 80),
				cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1);
		}

		if (imageTracker != nullptr && imageTracker->IsTracking())
		{
			cv::putText(frame, "OBJECT TRACKED!", cv::Point(30, 130),
				cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0), 2);
		}

		// Isolate matrix ownership explicitly before rendering
		cv::Mat displayFrame = frame.clone();
		cv::imshow(mainWindowName, displayFrame);

		if (motionDetector != nullptr && motionDetector->ShouldShowDebugMask())
		{
			cv::Mat debugMask;
			motionDetector->GetLatestThresholdMask(debugMask);
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

	isRunning = false;
}