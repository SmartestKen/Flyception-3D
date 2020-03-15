# Flyception-3D

> This project uses two low framerate real time cameras (low-speed cameras), galvometers and video-only high framerate camera (high-speed camera) to track small moving object (like LED) in constrained 3D space. 


## Hardware components
Here are photos that contain all of our hardware components.

![](header.png)


Notice that the chessboard is required to have the size of (a,b), where both a and b are odd numbers. We use 9x7 for our application. For explanation, please check **defOrigin.cpp** description below.


## Software components
Here is the data flow chart of our application.




| File/Folders | Purpose |
|--- | --- | 
| **Flyception_main.exe** | Main executable (can be run without Visual Studio, but please make sure all the configuration files are in the same places as they are in the github repo) |
| **Flyception_main.vcxproj**  | Main Visual Studio project, includes all SDK/include/library linker settings |
| (folder) fmfrecord | Standalone VS project used for generating chessboard images in 03/07/62/83 folders | 
| print_image_list.py | print the list of images within 03/07/62/83 folders to image_list xmls for convenience |
| (multiple) config xmls | store chessboard parameters for calibration. Needs to be manually modified every time you change to a new chessboard |
| **calibration.cpp** | Low-speed cameras calibration code. Relying on chessboard photos in folder 03/07/62/83 and config xmls and images_list xmls) | 
| (multiple) origin_ref jpg | Manually taken images that will be used to specify origin, x axis and y axis.
| **defOrigin.cpp** | Define the origin for the low speed cameras (and the entire system since galvo should be set to point to the same origin). Relying on origin_ref jpg, where origin is defined to be the center of the chessboard (which is why it needs to have odd sizes), x axis is defined to be the horizontal direction of the chessboard, y axis is defined to be the vertical direction of the chessboard. | 
| **camSetup.cpp** | subroutines for low-speed cameras setup, also contains important parameters like exposure time for Flyception_main.cpp | 
| **galvo_offset.cpp** | galvo calibration code and DAQ subroutines |
| Lx.m | Used to compute Lx (height of x axis mirror relative to the origin) which is necessary to determine how to convert from object positions to galvo signals. Ly can be computed similarly or use Ly = Lx - 1.5 (in centimeters) |
| **Flyception_main.cpp** | Entry point of the project/executable, which tracks the object using multithreading and OpenCV library| 
| Arena_Trigger.ino | Arduino trigger code that decides the acquisition frequency, can be modified even when the executables are running |
| video_analysis.m | Used to generate the figure in the result section based videos collected from high speed camera assuming its frame size is 1280*800 |
| | all others files are either generated, not used in the application process, or used as low level dependencies) |




## Simulation example and error plot
![Alt Text](https://github.com/SmartestKen/Flyception-3D/LED_trajectory_at_z0.gif)
![Alt Text](https://github.com/SmartestKen/Flyception-3D/LED_trajectory_above_z0.gif)

The only assumption of the algorithm is that the moving object has to be the much brighter than its surrounding. This assumption is made to compensate for our workstation CPU limitation (i7-7700, 4 cores, 8 threads). You can modify Flyception_main.cpp to perform contour detection if the assumption above is not suitable.


## Development dependencies
- Visual Studio 2019 Community (with vc140 tools and Windows 8.1 SDK)
- OpenCV 4.2.0 with OpenMP enabled
- Spinnaker Library (full version 1.27.0.48_x64)


## Bug reports/Feature suggestions/Inquiries
Please submit an issue or pull request. You can also reach Prof. Nicholas Gravish, the sponsor of this project, at ngravish@ucsd.edu.