## SplitFlap Monitoring Software
The **SplitFlap Monitoring Software** is responsable for:
- Determine if the split flap is rotating or if it's still
- Recognize the image shown while the split flap is still

### Components
The software consists of multiple components:\
-**LiveFeedManager**: manages a live feed capture from a camera device (webcam)\
-**VideoFeedManager**: manages a video stream feed from a pre-recorded video (.mp4 file)\
-**MotionDetector**: detects motion from the video feed (live or pre-recorded)\
-**ImageTracker**: tries to recognize the image shown while the split flap is still\
-**ConfigManager**: loads data from a **conf.json** to configure the application\
-**App**: the application class

### Dependencies
The project has only two dependencies:\
-**OpenCV**\
-**nlohmann json**

***Install OpenCV***<br>
1. Download [OpenCV installer](https://github.com/opencv/opencv/releases/download/5.0.0/opencv-5.0.0-windows.exe)
2. Run the installer and install it under **C:\opencv**

***Install nlohmann json***
1. Download this [file](https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp)
2. Put it wherever you want (e.g. C:\nlohmann\json.hpp)

### Visual Studio project update Include paths
To update include paths, right click on the project name in the **Solution Explorer** and click on **Properties**\
Go to **C/C++** -> **Additional Include Directories**, here you can update the include paths

### Visual Studio project update linker 
To update linker paths, right click on the project name in the **Solution Explorer** and click on **Properties**\
Go to **Linker** -> **General** -> **Additional Library Directories**, here you can update the libraries paths\
Go to **Linker** -> **Input** -> **Additional Depedencies**, here you can paste the libraries file names

### Console Logs
The application outputs its logs in two colors:\
- **Green output** is for monitoring logs such as the **motion state** (if it's rotaing or not) or if it has recognized an image
- **Cyan output** is for  app logs such as errors during the application runtime

### Log files
The application save its logs into two files:
- **App.log** contains only logs related to the application (errors, infos etc.)
- **Monitor.log** contains only logs related to the split flap monitoring so it easier to just audit if the split flap is rotating and if the image has been recognized everytime the split flap stops rotating

### Config values
**"isLiveFeed": false** -> the feed mode of the application, if it receives a live feed from a webcam or a pre-recorded feed from a video file

**"liveFeedWidth": 1280** -> the desired width of the live feed from a webcam (not an absolute value, it’s the webcam that decides which width it can return)

**"liveFeedHeight": 720** -> the desired height of the live feed from a webcam

**"videoFeedFile": "split_flap2.mp4"** -> the pre-recorded video to play in video feed mode

**"motionThreshold": 6000** -> the threshold to determine if a frame contains motion. Any frame with more changing pixels than this threshold value is said to contain motion

**"debounceMotionThreshold": 5** -> the number of consecutive frames containing motion required to trigger a "Rotating" state.
Increase this value to increase the motion detection stability

**"debounceStillThreshold": 5** -> the number of consecutive still frames required to trigger a "Not Rotating" state\

**"mog2History": 120** -> how many past frames the algorithm analyzes to build and mantain its background model.\
Instead of just comparing the current frame to a single previous frame, the MOG2 algorithm tracks the historical behavior of every individual pixel across this history window.\
This helps the system differentiate actual motion from random noise or environmental changes.\
The real-world time window it covers depends directly on the video's frame rate.\
For example, in this application (which targets 60 FPS), the default value of 120 means the algorithm is evaluating roughly the last 2 seconds of video footage to establish what the stationary background looks like.

**"mog2Threshold": 32** -> it dictates how radically a pixel's current color or brightness must depart from its historical average to be classified as active motion rather than static background or random noise.
- Lower Threshold Values: Make the detector highly sensitive. The algorithm will pick up very subtle pixel variations (such as minor shadows or camera sensor noise), but this increases the risk of false positives.
- Higher Threshold Values: Make the detector stricter. Only significant, high-contrast visual changes will be classified as motion, filtering out subtle ambient noise at the expense of potentially missing slow or low-contrast movements.

**"roi": {
		"x": 473,
		"y": 112,
		"width": 507,
		"height": 443
}** -> describes a region of interest in the feed.

**"imageTargets": [
		"SplitFlap.jpg",
		"Whale.jpg"
	]
}**: -> an array of images, each one representing the final picture each split flap shows when it's still.\
The software use this images to compare them with its video feed and tries to recognize them.
