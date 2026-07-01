## SplitFlap Monitoring Software
The **SplitFlap Monitoring Software** is responsable for:
- Determine if the split flap is rotating or if it's still
- Recognize the image shown while the split flap is still

### Components
The software consists of multiple components.
-**LiveFeedManager**: manages a live feed capture from a camera device (webcam)
-**VideoFeedManager**: manages a video stream feed from a pre-recorded video (.mp4 file)
-**MotionDetector**: detects motion from the video feed (live or pre-recorded)
-**ImageTracker**: tries to recognize the image shown while the split flap is still
-**ConfigManager**: loads data from a **conf.json** to configure the application
-**App**: the application class

### Dependencies
The project has only two dependencies:
-**OpenCV**
-**nlohmann json**

***Install OpenCV***
1.Download [OpenCV installer](https://github.com/opencv/opencv/releases/download/5.0.0/opencv-5.0.0-windows.exe)
2.Run the installer and install it under **C:\opencv**

***Install nlohmann json***
1. Download this [file](https://github.com/nlohmann/json/releases/download/v3.12.0/json.hpp)
2. Put it wherever you want (e.g. C:\nlohmann\json.hpp)

### Visual Studio project update Include paths
To update include paths, right click on the project name in the **Solution Explorer** and click on **Properties**
Go to **C/C++** -> **Additional Include Directories**, here you can update the include paths

### Visual Studio project update linker 
To update linker paths, right click on the project name in the **Solution Explorer** and click on **Properties**
Go to **Linker** -> **General** -> **Additional Library Directories**, here you can update the libraries paths
Go to **Linker** -> **Input** -> **Additional Depedencies**, here you can paste the libraries file names

### Console Logs
The application outputs its logs in two colors:
**Green output** is for monitoring logs such as the **motion state** (if it's rotaing or not) or if it has recognized an image
**Cyan output** is for  app logs such as errors during the application runtime

### Log files
The application save its logs into two files:
**App.log** contains only logs related to the application (errors, infos etc.)
**Monitor.log** contains only logs related to the split flap monitoring so it easier to just audit if the split flap is rotating and if the image has been recognized everytime the split flap stops rotating

