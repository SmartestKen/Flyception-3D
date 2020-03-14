// fmfrecord.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"


using namespace std;
using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace cv;
using namespace moodycamel;

#define MAXFRAMES 100

bool stream = true;

// specify trigger type
enum triggerType
{
	SOFTWARE,
	HARDWARE
};
const triggerType chosenTrigger = HARDWARE;

// specify exposu, frame rate, image size 
double exposureTimeToSet = 4000.0; // in microseconds (original 2000.0
double FrameRateToSet = 100; // (only for software trigger)
int imageWidth = 1080, imageHeight = 1080;

bool lrecord = false;
bool rrecord = false;
bool blrecord = false;
bool brrecord = false;

ReaderWriterQueue<ImagePtr> lq(100), rq(100), blq(100), brq(100);

ReaderWriterQueue<ImagePtr> lrec(MAXFRAMES);
ReaderWriterQueue<ImagePtr> rrec(MAXFRAMES);
ReaderWriterQueue<ImagePtr> blrec(MAXFRAMES);
ReaderWriterQueue<ImagePtr> brrec(MAXFRAMES);

ReaderWriterQueue<Mat> ldisp_frame(1), rdisp_frame(1), bldisp_frame(1), brdisp_frame(1);

int llast = 0, lfps = 0;
int rlast = 0, rfps = 0;
int bllast = 0, blfps = 0;
int brlast = 0, brfps = 0;



int ConvertTimeToFPS(uint64_t ctime, uint64_t ltime)
{
	int dtime = 0;
	// 1000000000 nanoseconds = 1 second

	dtime = ctime - ltime;
	dtime = 1000000000 / dtime;
	return dtime;
}

class ImageEventHandler : public ImageEvent
{
public:
	// The constructor retrieves the serial number and initializes the image counter to 0.
	ImageEventHandler(CameraPtr pCam)
	{
		// Retrieve device serial number
		INodeMap& nodeMap = pCam->GetTLDeviceNodeMap();

		m_deviceSerialNumber = "";
		CStringPtr ptrDeviceSerialNumber = nodeMap.GetNode("DeviceSerialNumber");
		if (IsAvailable(ptrDeviceSerialNumber) && IsReadable(ptrDeviceSerialNumber))
		{
			m_deviceSerialNumber = ptrDeviceSerialNumber->GetValue();
		}

		// Initialize image counter to 0
		m_imageCnt = 0;

		// Release reference to camera
		pCam = nullptr;
	}
	~ImageEventHandler()
	{
	}

	void OnImageEvent(ImagePtr image)
	{
		if (image->IsIncomplete())
		{
			std::cout << "Image incomplete with image status " << image->GetImageStatus() << "..." << endl << endl;
		}
		else
		{
			// Convert image to mono 8
			ImagePtr convertedImage = image->Convert(PixelFormat_Mono8, HQ_LINEAR);
			
			//lfps = ConvertTimeToFPS(image->GetTimeStamp(), llast); // TimeStamp is in nanoseconds
			//llast = image->GetTimeStamp();
			
			if (stream) {
				if (m_deviceSerialNumber == "19312783") {
					lq.enqueue(convertedImage);
					//std::cout << "lq queued. size: " << lq.size_approx() << endl;
				}
				else if (m_deviceSerialNumber == "19340262") rq.enqueue(convertedImage);
				else if (m_deviceSerialNumber == "19346503") blq.enqueue(convertedImage);
				else if (m_deviceSerialNumber == "19305407") brq.enqueue(convertedImage);
				else std::cout << "***device serial number not found!" << endl;

				m_imageCnt++;
			}

			return;
			std::cout << "got here" << endl;
		}
	}

	// Getter for image counter
	int getImageCount()
	{
		return m_imageCnt;
	}

private:
	unsigned int m_imageCnt;
	string m_deviceSerialNumber;
};



//void OnLeftImageGrabbed(CameraPtr pCam) // Image* pImage, const void* pCallbackData)
//{
//	ImagePtr pResultImage = pCam->GetNextImage();
//	if (pResultImage->IsIncomplete())
//	{
//		// Retrieve and print the image status description
//		std::cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
//			<< "..." << endl
//			<< endl;
//	}
//	ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
//	//std::cout << "\tTimestamp: " << pResultImage->GetTimeStamp() << endl;
//	lfps = ConvertTimeToFPS(pResultImage -> GetTimeStamp(), llast); // TimeStamp is in nanoseconds
//	llast = pResultImage->GetTimeStamp();
//	//img.DeepCopy(pImage);
//	
//	if (stream) {
//		lq.enqueue(convertedImage);
//		std::cout << "\tlq queued" << endl;
//	}
//	
//	pResultImage->Release();
//	return;
//	std::cout << "got here" << endl;
//}
//
//void OnRightImageGrabbed(CameraPtr pCam) 
//{
//	ImagePtr pResultImage = pCam->GetNextImage();
//	if (pResultImage->IsIncomplete())
//	{
//		// Retrieve and print the image status description
//		std::cout << "Image incomplete: " << Image::GetImageStatusDescription(pResultImage->GetImageStatus())
//			<< "..." << endl
//			<< endl;
//	}
//	ImagePtr convertedImage = pResultImage->Convert(PixelFormat_Mono8, HQ_LINEAR);
//	//std::cout << "\tTimestamp: " << pResultImage->GetTimeStamp() << " -- stream: " << stream << endl;
//	rfps = ConvertTimeToFPS(pResultImage->GetTimeStamp(), rlast); // TimeStamp is in nanoseconds
//	rlast = pResultImage->GetTimeStamp();
//	if (stream) {
//		rq.enqueue(convertedImage);
//		std::cout << "\trq queued" << endl; 
//	}
//
//	pResultImage->Release();
//	return;
//}
////
////void OnRightImageGrabbed(Image* pImage, const void* pCallbackData)
////{
////	Image img;
////
////	rfps = ConvertTimeToFPS(pImage->GetTimeStamp().cycleCount, rlast);
////	rlast = pImage->GetTimeStamp().cycleCount;
////
////	img.DeepCopy(pImage);
////	
////	if (stream)
////		rq.enqueue(img);
////
////	return;
////}
////
////void OnBLeftImageGrabbed(Image* pImage, const void* pCallbackData)
////{
////	Image img;
////	
////	blfps = ConvertTimeToFPS(pImage->GetTimeStamp().cycleCount, bllast);
////	bllast = pImage->GetTimeStamp().cycleCount;
////
////	img.DeepCopy(pImage);
////	
////	if (stream)
////		lq.enqueue(img);
////	
////	return;
////}
////
////void OnBRightImageGrabbed(Image* pImage, const void* pCallbackData)
////{
////	Image img;
////
////	brfps = ConvertTimeToFPS(pImage->GetTimeStamp().cycleCount, brlast);
////	brlast = pImage->GetTimeStamp().cycleCount;
////
////	img.DeepCopy(pImage);
////	
////	if (stream)
////		rq.enqueue(img);
////
////	return;
////}
//

// function to print device info or error that cannot be reached
int PrintDeviceInfo(INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
	int result = 0;

	gcstring deviceSerialNumber("");
	CStringPtr ptrStringSerial = nodeMapTLDevice.GetNode("DeviceSerialNumber");
	if (IsAvailable(ptrStringSerial) && IsReadable(ptrStringSerial))
	{
		deviceSerialNumber = ptrStringSerial->GetValue();

		std::cout << endl << "*** Configuring, Initializing, and Starting Acquistion for Camera:  " << deviceSerialNumber << " ***" << endl;
	}

	//featurelist_t features;
	//ccategoryptr category = nodemap.getnode("deviceinformation");
	//if (isavailable(category) && isreadable(category))
	//{
	//	category->getfeatures(features);
	//	featurelist_t::const_iterator it;
	//	for (it = features.begin(); it != features.end(); ++it)
	//	{
	//		cnodeptr pfeaturenode = *it;
	//		std::cout << pfeaturenode->getname() << " : ";
	//		cvalueptr pvalue = (cvalueptr)pfeaturenode;
	//		std::cout << (isreadable(pvalue) ? pvalue->tostring() : "node not readable");
	//		std::cout << endl;
	//	}
	//}
	else
	{
		std::cout << "Device control information not available." << endl;
	}
	return result;
}


// function to configure trigger
int ConfigureTrigger(INodeMap& nodeMap, INodeMap& nodeMapTLDevice)
{
	int result = 0;

	if (chosenTrigger == SOFTWARE)
	{
		std::cout << "Software trigger chosen..." << endl;
	}
	else if (chosenTrigger == HARDWARE)
	{
		std::cout << "Hardware trigger chosen..." << endl;
	}

	try
	{
		// Ensure trigger mode off
		CEnumerationPtr ptrTriggerMode = nodeMap.GetNode("TriggerMode");
		if (!IsAvailable(ptrTriggerMode) || !IsReadable(ptrTriggerMode))
		{
			std::cout << "Unable to disable trigger mode (node retrieval). Aborting..." << endl;
			return -1;
		}
		CEnumEntryPtr ptrTriggerModeOff = ptrTriggerMode->GetEntryByName("Off");
		if (!IsAvailable(ptrTriggerModeOff) || !IsReadable(ptrTriggerModeOff))
		{
			std::cout << "Unable to disable trigger mode (enum entry retrieval). Aborting..." << endl;
			return -1;
		}
		ptrTriggerMode->SetIntValue(ptrTriggerModeOff->GetValue());
		std::cout << "Trigger mode disabled..." << endl;


		// Set trigger to hardware
		CEnumerationPtr ptrTriggerSource = nodeMap.GetNode("TriggerSource");
		if (!IsAvailable(ptrTriggerSource) || !IsWritable(ptrTriggerSource))
		{
			std::cout << "Unable to set trigger mode (node retrieval). Aborting..." << endl;
			return -1;
		}
		if (chosenTrigger == SOFTWARE)
		{
			// Set trigger mode to software
			CEnumEntryPtr ptrTriggerSourceSoftware = ptrTriggerSource->GetEntryByName("Line3");
			if (!IsAvailable(ptrTriggerSourceSoftware) || !IsReadable(ptrTriggerSourceSoftware))
			{
				std::cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
				return -1;
			}
			ptrTriggerSource->SetIntValue(ptrTriggerSourceSoftware->GetValue());
			std::cout << "Trigger source set to software..." << endl;
		}
		else if (chosenTrigger == HARDWARE)
		{
			// Set trigger mode to hardware ('Line0') normally
			CEnumEntryPtr ptrTriggerSourceHardware = ptrTriggerSource->GetEntryByName("Line0");
			if (!IsAvailable(ptrTriggerSourceHardware) || !IsReadable(ptrTriggerSourceHardware))
			{
				std::cout << "Unable to set trigger mode (enum entry retrieval). Aborting..." << endl;
				return -1;
			}
			ptrTriggerSource->SetIntValue(ptrTriggerSourceHardware->GetValue());
			std::cout << "Trigger source set to hardware..." << endl;
		}


		// Set trigger overlap to "Readout"
		CEnumerationPtr ptrTriggerOverlap = nodeMap.GetNode("TriggerOverlap");
		if (!IsAvailable(ptrTriggerOverlap) || !IsReadable(ptrTriggerOverlap))
		{
			std::cout << "Unable to find trigger overlap (node retrieval). Aborting..." << endl;
			return -1;
		}
		CEnumEntryPtr ptrTriggerOverlapRO = ptrTriggerOverlap->GetEntryByName("ReadOut");
		if (!IsAvailable(ptrTriggerOverlapRO) || !IsReadable(ptrTriggerOverlapRO))
		{
			std::cout << "Unable to set trigger overlap (enum entry retrieval). Aborting..." << endl;
			return -1;
		}
		ptrTriggerOverlap->SetIntValue(ptrTriggerOverlapRO->GetValue());
		std::cout << "Trigger overlap set to Read Out..." << endl;


		// Set width and height
		CIntegerPtr ptrWidth = nodeMap.GetNode("Width");
		if (IsAvailable(ptrWidth) && IsWritable(ptrWidth))
		{
			ptrWidth->SetValue(imageWidth);
			std::cout << "Width set to " << ptrWidth->GetValue() << "..." << endl;
		}
		else
		{
			std::cout << "Width not available..." << endl;
		}
		CIntegerPtr ptrHeight = nodeMap.GetNode("Height");
		if (IsAvailable(ptrHeight) && IsWritable(ptrHeight))
		{
			ptrHeight->SetValue(imageHeight);
			std::cout << "Height set to " << ptrHeight->GetValue() << "..." << endl ;
		}
		else
		{
			std::cout << "Height not available..." << endl;
		}


		// Set frame rate to 100
		CFloatPtr ptrFrameRate = nodeMap.GetNode("AcquisitionFrameRate");
		const double FrameRateMax = ptrFrameRate->GetMax();
		if (IsAvailable(ptrFrameRate) && IsWritable(ptrFrameRate))
		{
			if (FrameRateToSet > FrameRateMax)
			{
				FrameRateToSet = FrameRateMax;
			}
			ptrFrameRate->SetValue(FrameRateToSet);
			//ptrFrameRate->SetValue(ptrFrameRate->GetMax());
			std::cout << "Set frame rate to " << ptrFrameRate->GetValue() << "..." << endl;
		}
		else
		{
			std::cout << "Frame rate control not available..." << endl;
		}


		// Set Exposure
		CFloatPtr ptrExposureTime = nodeMap.GetNode("ExposureTime");
		if (!IsAvailable(ptrExposureTime) || !IsWritable(ptrExposureTime))
		{
			std::cout << "Unable to set exposure time. Aborting..." << endl << endl;
			return -1;
		}
		// Ensure desired exposure time does not exceed the maximum
		const double exposureTimeMax = ptrExposureTime->GetMax();
		if (exposureTimeToSet > exposureTimeMax)
		{
			exposureTimeToSet = exposureTimeMax;
		}
		ptrExposureTime->SetValue(exposureTimeToSet);
		std::cout << "Exposure time set to " << exposureTimeToSet << " us..." << endl ;

		// Turn trigger mode on
		CEnumEntryPtr ptrTriggerModeOn = ptrTriggerMode->GetEntryByName("On");
		if (!IsAvailable(ptrTriggerModeOn) || !IsReadable(ptrTriggerModeOn))
		{
			std::cout << "Unable to enable trigger mode (enum entry retrieval). Aborting..." << endl;
			return -1;
		}
		ptrTriggerMode->SetIntValue(ptrTriggerModeOn->GetValue());
		std::cout << "Trigger mode turned back on..." << endl ;
		//}

	}
	catch (Spinnaker::Exception & e)
	{
		std::cout << "Error: " << e.what() << endl;
		result = -1;
	}
	return result;
}


// make each cam start acquiring images
int StartAcquisition(CameraList camList) {
	int result = 0;
	CameraPtr pCam = nullptr;
	std::cout << endl << "*** Starting acquisition for cameras..." << endl;
	try
	{
		for (unsigned int i = 0; i < camList.GetSize(); i++)
		{
			// Select camera
			pCam = camList.GetByIndex(i);
			// Retrieve TL device nodemap
			INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
			// Retrieve GenICam nodemap
			INodeMap& nodeMap = pCam->GetNodeMap(); // cannot get these unless cam is initialized


			// Set acquisition to continuous
			CEnumerationPtr ptrAcquisitionMode = nodeMap.GetNode("AcquisitionMode");
			if (!IsAvailable(ptrAcquisitionMode) || !IsWritable(ptrAcquisitionMode))
			{
				std::cout << "Unable to set acquisition mode to continuous (enum retrieval). Aborting..." << endl << endl;
				return -1;
			}
			CEnumEntryPtr ptrAcquisitionModeContinuous = ptrAcquisitionMode->GetEntryByName("Continuous");
			if (!IsAvailable(ptrAcquisitionModeContinuous) || !IsReadable(ptrAcquisitionModeContinuous))
			{
				std::cout << "Unable to set acquisition mode to continuous (entry retrieval). Aborting..." << endl << endl;
				return -1;
			}
			const int64_t acquisitionModeContinuous = ptrAcquisitionModeContinuous->GetValue();
			ptrAcquisitionMode->SetIntValue(acquisitionModeContinuous);

			// Start acquring images into camera memory
			pCam->BeginAcquisition();
			std::cout << "Acquisition set to continuous and started acquiring images for cam: " << i << endl;
		}
	}
	catch (Spinnaker::Exception & e)
	{
		std::cout << "Error: " << e.what() << endl;
		result = -1;
	}
	return result;
}


// prepare and register an image event handler for each camera
int ConfigureImageEvents(CameraPtr pCam, ImageEventHandler*& imageEventHandler)
{
	int result = 0;
	try
	{
		// Create image event
		imageEventHandler = new ImageEventHandler(pCam);
		// Register image event handler
		pCam->RegisterEvent(*imageEventHandler);
	}
	catch (Spinnaker::Exception & e)
	{
		std::cout << "Error: " << e.what() << endl;
		result = -1;
	}
	return result;
}

// for each camera: print info, initialize, configure trigger and settings
int InitializeMultipleCameras(CameraList camList) {
	int result = 0;
	CameraPtr pCam = nullptr;
	try
	{
		for (unsigned int i = 0; i < camList.GetSize(); i++)
		{
			int err = 0;

			// Select camera
			pCam = camList.GetByIndex(i);
			// Retrieve TL device nodemap
			INodeMap& nodeMapTLDevice = pCam->GetTLDeviceNodeMap();
			// Initialize camera
			pCam->Init();
			// Retrieve GenICam nodemap
			INodeMap& nodeMap = pCam->GetNodeMap(); // cannot get these unless cam is initialized
			// Print device information
			result = PrintDeviceInfo(nodeMap, nodeMapTLDevice);
			// Configure trigger
			err = ConfigureTrigger(nodeMap, nodeMapTLDevice);
			
			if (err < 0)
			{
				return err;
			}
		}
	}
	catch (Spinnaker::Exception & e)
	{
		std::cout << "Error: " << e.what() << endl;
		result = -1;
	}
	return result;
}



int _tmain(int argc, _TCHAR* argv[])
{
	std::cout << "WELCOME TO FLYCEPTION 3D"<< endl << endl;
	//PGRcam lcam, rcam, blcam, brcam; DO THIS BY HAND INSTEAD OF A CLASS LIKE IN FLYCAPTURE

	// check that have access to folder
	FILE* tempFile;
	errno_t err;
	err = fopen_s(&tempFile, "C:\\test.txt", "w+");
	if (tempFile == nullptr)
	{
		std::cout << "Failed to create file in current folder.  Please check "
			"permissions."
			<< endl;
		std::cout << "Press Enter to exit..." << endl;
		//getchar();
		return -1;
	}
	fclose(tempFile);
	remove("test.txt");

	int result = 0;
	CameraPtr pCam = nullptr;

	// Retrieve singleton reference to system object
	SystemPtr system = System::GetInstance();

	// Retrieve list of cameras from the system
	CameraList camList = system->GetCameras();
	unsigned int numCameras = camList.GetSize();
	std::cout << "Number of cameras detected: " << numCameras << endl << endl;

	// Close session if fewer than 4 cameras
	if (numCameras < 4)
	{
		// Clear camera list before releasing system
		camList.Clear();
		// Release system
		system->ReleaseInstance();
		std::cout << "Not enough cameras!" << endl;
		std::cout << "Done! Press Enter to exit..." << endl;
		//getchar();
		Sleep(100000000);
		return -1;
	}

	// Initialize and set trigger mode of each camera
	result = result | InitializeMultipleCameras(camList);

	// Configure Image Event Handler (can't figure out how to set the event handlers in a for loop :-/ )
	ImageEventHandler* imageEventHandler0;
	ImageEventHandler* imageEventHandler1;
	ImageEventHandler* imageEventHandler2;
	ImageEventHandler* imageEventHandler3;
	pCam = camList.GetByIndex(0);
	err = ConfigureImageEvents(pCam, imageEventHandler0);
	pCam = camList.GetByIndex(1);
	err = ConfigureImageEvents(pCam, imageEventHandler1);
	pCam = camList.GetByIndex(2);
	err = ConfigureImageEvents(pCam, imageEventHandler2);
	pCam = camList.GetByIndex(3);
	err = ConfigureImageEvents(pCam, imageEventHandler3);
	if (err < 0) return err;

	// start acquiring images on each camera
	result = result | StartAcquisition(camList); 
	if (result < 0) return result;
	//}
	//catch (Spinnaker::Exception & e)
	//{
	//	std::cout << "Error: " << e.what() << endl;
	//	result = -1;
	//}
	//	

	Sleep(3000); // give the cameras some time to start up


	// START READING, DISPLAYING, AND RECORDING IMAGES IN PARALLEL
	FmfWriter lout, rout, blout, brout;
	int lcount = 0, rcount = 0, blcount = 0, brcount = 0;

	#pragma omp parallel sections num_threads(13)
		{
		#pragma omp section
			{
				ImagePtr img;
				Mat frame;
				int err = 0;

				while (true)
				{
					if (lq.try_dequeue(img))
					{
						lfps = ConvertTimeToFPS(img->GetTimeStamp(), llast); // TimeStamp is in nanoseconds
						llast = img->GetTimeStamp();
						unsigned int rowBytes = img -> GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
						Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);
						frame = tframe.clone();
						putText(frame, to_string(lfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));

						if (lrecord)
						{
							putText(frame, to_string(lcount), Point(0, 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							lrec.enqueue(img);
							lcount++;
						}

						ldisp_frame.try_enqueue(frame.clone());
					}

					if (!stream)
						break;
				}
			}
		#pragma omp section
			{
				ImagePtr img;
				Mat frame;
				while (true)
				{
					if (rq.try_dequeue(img))
					{
						rfps = ConvertTimeToFPS(img->GetTimeStamp(), rlast); // TimeStamp is in nanoseconds
						rlast = img->GetTimeStamp();
						unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
						Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);
						frame = tframe.clone();
						putText(frame, to_string(rfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));

						if (rrecord)
						{
							putText(frame, to_string(rcount), Point(0, 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							rrec.enqueue(img);
							rcount++;
						}
						rdisp_frame.try_enqueue(frame.clone());
					}
					if (!stream)
						break;
				}
			}
		#pragma omp section
			{
				ImagePtr img;
				Mat frame;
				while (true)
				{
					if (blq.try_dequeue(img))
					{
						blfps = ConvertTimeToFPS(img->GetTimeStamp(), bllast); // TimeStamp is in nanoseconds
						bllast = img->GetTimeStamp();
						unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
						Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);
						frame = tframe.clone();
						putText(frame, to_string(blfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));

						if (blrecord)
						{
							putText(frame, to_string(blcount), Point(0, 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							blrec.enqueue(img);
							blcount++;
						}

						bldisp_frame.try_enqueue(frame.clone());
					}
					if (!stream)
						break;
				}
			}
		
		#pragma omp section
			{
				ImagePtr img;
				Mat frame;
				while (true)
				{
					if (brq.try_dequeue(img))
					{
						brfps = ConvertTimeToFPS(img->GetTimeStamp(), brlast); // TimeStamp is in nanoseconds
						brlast = img->GetTimeStamp();
						unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
						Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);
						frame = tframe.clone();
						putText(frame, to_string(brfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));

						if (brrecord)
						{
							putText(frame, to_string(brcount), Point(0, 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							brrec.enqueue(img);
							brcount++;
						}
						brdisp_frame.try_enqueue(frame.clone());
					}
					if (!stream)
						break;
				}
			}





	 	#pragma omp section
	 	{
	 		ImagePtr tImage;
			int l_count = 0;
	 		while (true)
	 		{
				//std::cout << "lrec size: " << lrec.size_approx() << endl;
	 			if (lrec.try_dequeue(tImage))
	 			{
					/*
	 				if (!lout.IsOpen())
	 				{
	 					lout.id = 19312783;
	 					lout.Open();
	 					lout.InitHeader(imageWidth, imageHeight);
	 					lout.WriteHeader();
	 				}
	 				lout.WriteFrame(tImage);
	 				lout.WriteLog(tImage);
					lout.nframes++;
					*/
					char buffer[100];
					
					sprintf_s(buffer, "C:/Users/Administrator/Desktop/Flyception_main/83/lout_%d.jpg", l_count);
					tImage->Save(buffer, JPEG);
					l_count++;
	 				
	 			}
	 			else
	 			{
	 				if (!lrecord && lout.IsOpen())
	 					lout.Close();

	 				if (!stream)
	 					break;
	 			}
	 		}
	 	}
		#pragma omp section
		{
			ImagePtr tImage;
			int r_count = 0;
			while (true)
			{
				if (rrec.try_dequeue(tImage))
				{
					/*
					if (!rout.IsOpen())
					{
						rout.id = 19340262;
						rout.Open();
						rout.InitHeader(imageWidth, imageHeight);
						rout.WriteHeader();
					}
					rout.WriteFrame(tImage);
					rout.WriteLog(tImage);
					rout.nframes++;
					*/
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/Desktop/Flyception_main/62/rout_%d.jpg", r_count);
					tImage->Save(buffer, JPEG);
					r_count++;

				}
				else
				{
					if (!rrecord && rout.IsOpen())
						rout.Close();
					if (!stream)
						break;
				}
			}
		}
		#pragma omp section
		{
			ImagePtr tImage;
			int bl_count = 0;
			while (true)
			{
				if (blrec.try_dequeue(tImage))
				{
					/*
					if (!blout.IsOpen())
					{
						blout.id = 19346503;
						blout.Open();
						blout.InitHeader(imageWidth, imageHeight);
						blout.WriteHeader();
					}
					blout.WriteFrame(tImage);
					blout.WriteLog(tImage);
					blout.nframes++;
					*/
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/Desktop/Flyception_main/03/blout_%d.jpg", bl_count);
					tImage->Save(buffer, JPEG);
					bl_count++;


				}
				else
				{
					if (!blrecord && blout.IsOpen())
						blout.Close();
					if (!stream)
						break;
				}
			}
		}
		#pragma omp section
		{
			ImagePtr tImage;
			int br_count = 0;
			while (true)
			{
				if (brrec.try_dequeue(tImage))
				{
					/*
					if (!brout.IsOpen())
					{
						brout.id = 19305407;
						brout.Open();
						brout.InitHeader(imageWidth, imageHeight);
						brout.WriteHeader();
					}
					brout.WriteFrame(tImage);
					brout.WriteLog(tImage);
					brout.nframes++;
					*/
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/Desktop/Flyception_main/07/brout_%d.jpg", br_count);
					tImage->Save(buffer, JPEG);
					br_count++;
				}
				else
				{
					if (!brrecord && brout.IsOpen())
						brout.Close();
					if (!stream)
						break;
				}
			}
		}
	 	#pragma omp section
	 		{
	 			Mat frame;
	 			while (true)
	 			{
					//std::cout << "ldisp size: " << ldisp_frame.size_approx() << endl;
	 				if (ldisp_frame.try_dequeue(frame))
	 					imshow("camera left", frame);
	 				waitKey(1);
	 				if (!stream)
	 					break;
	 			}
	 		}
		#pragma omp section
			{
				Mat frame;
				while (true)
				{
					if (rdisp_frame.try_dequeue(frame))
						imshow("camera right", frame);
					waitKey(1);
					if (!stream)
						break;
				}
			}
		#pragma omp section
			{
				Mat frame;
				while (true)
				{
					if (bldisp_frame.try_dequeue(frame))
						imshow("camera back left", frame);
					waitKey(1);
					if (!stream)
						break;
				}
			}
		#pragma omp section
			{
				Mat frame;
				while (true)
				{
					if (brdisp_frame.try_dequeue(frame))
						imshow("camera back right", frame);
					waitKey(1);
					if (!stream)
						break;
				}
			}
	 	#pragma omp section
	 	{
			int record_key_state = 0;

	 		while (true)
	 		{
	 			if (GetAsyncKeyState(VK_F2))
	 			{
	 				if (!record_key_state)
	 				{
	 					lrecord = !lrecord;
	 					rrecord = !rrecord;
						blrecord = !blrecord;
						brrecord = !brrecord;

	 					lcount = 0;
	 					rcount = 0;
						blcount = 0;
						brcount = 0;
	 				}
	 				record_key_state = 1;
	 			}
	 			else
	 				record_key_state = 0;

	 			if (lrecord)
	 			{
	 				if (lcount == MAXFRAMES)
	 				{
	 					lcount = 0;
	 					lrecord = false;
	 				}
	 			}
	 			if (rrecord)
	 			{
	 				if (rcount == MAXFRAMES)
	 				{
	 					rcount = 0;
	 					rrecord = false;
	 				}
	 			}
				if (blrecord)
				{
					if (blcount == MAXFRAMES)
					{
						blcount = 0;
						blrecord = false;
					}
				}
				if (brrecord)
				{
					if (brcount == MAXFRAMES)
					{
						brcount = 0;
						brrecord = false;
					}
				}

	 			if (GetAsyncKeyState(VK_ESCAPE))
	 			{
	 				stream = false;
	 				break;
	 			}
	 		}
	 	}
		}

	 if (lout.IsOpen())
	 	lout.Close();
	 if (rout.IsOpen())
	 	rout.Close();
	 if (blout.IsOpen())
		 blout.Close();
	 if (brout.IsOpen())
		 brout.Close();


	// Deinitialize each camera
	std::cout << endl << "*** Shutting down system ***" << endl;

	try
	{
		// stop acquisition
		for (unsigned int i = 0; i < camList.GetSize(); i++)
		{
			// Select camera
			pCam = camList.GetByIndex(i);
			// Stop acquisition
			pCam->EndAcquisition();
		}
		// Release image event handlers
		pCam = camList.GetByIndex(0);
		pCam->UnregisterEvent(*imageEventHandler0);
		pCam = camList.GetByIndex(1);
		pCam->UnregisterEvent(*imageEventHandler1);
		pCam = camList.GetByIndex(2);
		pCam->UnregisterEvent(*imageEventHandler2);
		pCam = camList.GetByIndex(3);
		pCam->UnregisterEvent(*imageEventHandler3);
		delete imageEventHandler0;
		delete imageEventHandler1;
		delete imageEventHandler2;
		delete imageEventHandler3;
		// deinitialize
		for (unsigned int i = 0; i < camList.GetSize(); i++)
		{
			// Select camera
			pCam = camList.GetByIndex(i);
			// Deinitialize camera
			pCam->DeInit();
			std::cout << "Stopped acquisition and deinitialized cam: " << i << endl;
		}
	}
	catch (Spinnaker::Exception & e)
	{
		std::cout << "Error: " << e.what() << endl;
		result = -1;
	}

	// Clear camera list before releasing system
	camList.Clear();

	// Release system
	system->ReleaseInstance();

	//std::cout << endl << "Done! Press Enter to exit..." << endl;
	//getchar(); // USE IF DON'T WANT TO AUTOMATICALLY CLOSE

	return 0; // result;

}


