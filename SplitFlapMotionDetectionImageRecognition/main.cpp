#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 1. Initialize the webcam (0 is usually the default built-in camera)
    cv::VideoCapture cap(0);

    // Check if the webcam opened successfully
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open the webcam!" << std::endl;
        return -1;
    }

    // Create a window to display the live feed
    std::string windowName = "Live Webcam Feed - Press ESC to Exit";
    cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);

    cv::Mat frame;

    std::cout << "Streaming live video... Press 'ESC' to quit." << std::endl;

    // 2. Main loop to grab and show frames
    while (true) {
        // Capture a new frame from the webcam
        cap >> frame;

        // If the frame is empty, break the loop
        if (frame.empty()) {
            std::cerr << "Error: Blank frame grabbed." << std::endl;
            break;
        }

        // Display the frame in the created window
        cv::imshow(windowName, frame);

        // 3. Wait for 30ms and check if the user pressed the ESC key (ASCII 27)
        char key = (char)cv::waitKey(30);
        if (key == 27) {
            break;
        }
    }

    // 4. Release the camera resources and close windows
    cap.release();
    cv::destroyAllWindows();

    return 0;
}