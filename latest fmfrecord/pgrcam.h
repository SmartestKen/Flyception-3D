#ifndef PGRCAM_H
#define PGRCAM_H

using namespace std;
//using namespace FlyCapture2;
using namespace Spinnaker;
using namespace cv;

class PGRcam
{
private:
	//CameraInfo camInfo; // for flycap
	ChunkData camInfo;
	//CameraBase base; // spin but not sure we need it?

	Image rawImage, convertedImage;
	
	//TimeStamp timestamp;
	uint64_t timestamp;

	//FlyCapture2::Error error;
	Spinnaker::Error error;
	

	//Mode k_fmt7Mode;
	enum k_fmt7Mode;
	// PixelFormat k_fmt7PixFmt;
	Spinnaker::PixelFormatEnums k_fmt7PixFmt;

	Format7Info fmt7Info;
	bool supported;

	Format7ImageSettings fmt7ImageSettings;
	Format7PacketInfo fmt7PacketInfo;

	TriggerModeInfo triggerModeInfo;
	TriggerMode triggerMode;

	bool valid;

public:

	Camera cam;
	int id;

	PGRcam();
	~PGRcam();




	// FlyCapture2::Error Connect(PGRGuid guid);
	// FlyCapture2::Error SetCameraParameters(int width, int height);
	// FlyCapture2::Error SetTrigger();
	// FlyCapture2::Error SetProperty(PropertyType type, float absValue);
	// //FlyCapture2::Error Start();
	// FlyCapture2::Error Stop();
	// FlyCapture2::Image GrabFrame();
	// Mat convertImagetoMat(Image img);
	// FlyCapture2::TimeStamp GetTimeStamp();
	// void GetImageSize(int &imageWidth, int &imageHeight);


	Spinnaker::Error Connect(PGRGuid guid);
	Spinnaker::Error SetCameraParameters(int width, int height);
	Spinnaker::Error SetTrigger();
	Spinnaker::Error SetProperty(PropertyType type, float absValue);
	Spinnaker::Error Stop();
	Spinnaker::Image GrabFrame();
	Mat convertImagetoMat(Image img);
	Spinnaker::TimeStamp GetTimeStamp();
	void GetImageSize(int &imageWidth, int &imageHeight);




};

#endif
