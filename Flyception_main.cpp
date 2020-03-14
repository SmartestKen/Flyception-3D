// 'Hello World!' program 
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <string>
#include <omp.h>
#include <queue>

#pragma once
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <vector>
#include <algorithm>
#include <conio.h>
#include <fstream>



#include "calibration.cpp"
#include "defOrigin.cpp"
#include "camSetup.cpp"


#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"


#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>

#include "826api.h"
#include "galvo_offset.cpp"
#include "readerwriterqueue.h"
#include "fmfwriter.cpp"
using namespace std;

using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace cv;
using namespace moodycamel;





Mat camMat[4];
Mat RTMat[4];
CameraPtr cameras[4];




// images from cameras
// ReaderWriterQueue<Image> imageQ[CAMERA_COUNT];
// ReaderWriterQueue<Image> maskQ[CAMERA_COUNT];
// ReaderWriterQueue<Point> pointQ[CAMERA_COUNT];

Mat triangulate_Linear_LS(Mat mat_P_l, Mat mat_P_r, Mat warped_back_l, Mat warped_back_r)
{
	Mat A(4, 3, CV_64FC1), b(4, 1, CV_64FC1), X(3, 1, CV_64FC1), X_homogeneous(4, 1, CV_64FC1), W(1, 1, CV_64FC1);
	W.at<double>(0, 0) = 1.0;
	A.at<double>(0, 0) = (warped_back_l.at<double>(0, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 0) - mat_P_l.at<double>(0, 0);
	A.at<double>(0, 1) = (warped_back_l.at<double>(0, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 1) - mat_P_l.at<double>(0, 1);
	A.at<double>(0, 2) = (warped_back_l.at<double>(0, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 2) - mat_P_l.at<double>(0, 2);
	A.at<double>(1, 0) = (warped_back_l.at<double>(1, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 0) - mat_P_l.at<double>(1, 0);
	A.at<double>(1, 1) = (warped_back_l.at<double>(1, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 1) - mat_P_l.at<double>(1, 1);
	A.at<double>(1, 2) = (warped_back_l.at<double>(1, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 2) - mat_P_l.at<double>(1, 2);
	A.at<double>(2, 0) = (warped_back_r.at<double>(0, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 0) - mat_P_r.at<double>(0, 0);
	A.at<double>(2, 1) = (warped_back_r.at<double>(0, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 1) - mat_P_r.at<double>(0, 1);
	A.at<double>(2, 2) = (warped_back_r.at<double>(0, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 2) - mat_P_r.at<double>(0, 2);
	A.at<double>(3, 0) = (warped_back_r.at<double>(1, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 0) - mat_P_r.at<double>(1, 0);
	A.at<double>(3, 1) = (warped_back_r.at<double>(1, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 1) - mat_P_r.at<double>(1, 1);
	A.at<double>(3, 2) = (warped_back_r.at<double>(1, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 2) - mat_P_r.at<double>(1, 2);
	b.at<double>(0, 0) = -((warped_back_l.at<double>(0, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 3) - mat_P_l.at<double>(0, 3));
	b.at<double>(1, 0) = -((warped_back_l.at<double>(1, 0) / warped_back_l.at<double>(2, 0)) * mat_P_l.at<double>(2, 3) - mat_P_l.at<double>(1, 3));
	b.at<double>(2, 0) = -((warped_back_r.at<double>(0, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 3) - mat_P_r.at<double>(0, 3));
	b.at<double>(3, 0) = -((warped_back_r.at<double>(1, 0) / warped_back_r.at<double>(2, 0)) * mat_P_r.at<double>(2, 3) - mat_P_r.at<double>(1, 3));

	solve(A, b, X, DECOMP_SVD);
	vconcat(X, W, X_homogeneous);

	//printf("[%f %f %f %f]\n", X_homogeneous.at<double>(0, 0), X_homogeneous.at<double>(1, 0), X_homogeneous.at<double>(2, 0), X_homogeneous.at<double>(3, 0));

	return X_homogeneous;
}



//TODO 1. lower down Hz to 50, remove display queue
// move executable to current dir to smoothly run.
int main(int argc, char* argv[])
{
   
	/*
    omp_set_num_threads(2);
    #pragma omp parallel for
    for (int i = 0; i < 2; i++)
    {
        std::string configFileName = "config_" + std::to_string(i+1) + ".xml";
        startCalibration(configFileName);
    }
    
	
    
    // merge after debugging
	
	
    for (int i = 0; i < 2; i++)
    {
        defOrigin(i);
        cout << "cam " << to_string(i) << " succeeds" << endl;
    }
	return 0;
	
	*/


	




    int initX = 34290;
    int initY = 29835;

	/*
    try {
        std::ifstream in("galvo_offset.txt");
        in >> initX >> initY;
    }
    catch (...)
    {
        cout << "offset file not found" << endl;
    }
	std::ofstream out("galvo_offset.txt");
	out << initX << " " << initY;
	*/

	// set to true to zero
    
    if (gToggle)
        galvo_offset(&initX, &initY);





	

    
    // open DAQ (for galvo) first
    S826_SystemOpen();


	std::cout << "WELCOME TO FLYCEPTION 3D" << endl << endl;
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


	for (int i = 0; i < 4; i++) {
		FileStorage fs_in("intrinsic_para_" + to_string(i + 1) + ".xml", FileStorage::READ);
		FileStorage fs_ex("extrinsic_para_" + to_string(i + 1) + ".xml", FileStorage::READ);

		fs_in["camera_matrix"] >> camMat[i];
		fs_ex["rtMat"] >> RTMat[i];


		fs_in.release();
		fs_ex.release();
	}




	


	int result = 0;
	CameraPtr pCam = nullptr;

	// Retrieve singleton reference to system object
	SystemPtr system = System::GetInstance();

	// Retrieve list of cameras from the system
	CameraList camList = system->GetCameras();
	unsigned int numCameras = camList.GetSize();
	std::cout << "Number of cameras detected: " << numCameras << endl << endl;
	// std::cout << getBuildInformation() << endl;
	// Close session if fewer than 4 cameras
	if (numCameras < 2)
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



	// remove the two useless cameras
	/*
	pCam = camList.GetByIndex(2);
	err = ConfigureImageEvents(pCam, imageEventHandler2);
	pCam = camList.GetByIndex(3);
	err = ConfigureImageEvents(pCam, imageEventHandler3);
	*/


	if (err < 0) return err;
	
	// start acquiring images on each camera
	result = result | StartAcquisition(camList);
	if (result < 0) return result;


	Sleep(1000); // give the cameras some time to start up


	// START READING, DISPLAYING, AND RECORDING IMAGES IN PARALLEL
	// FmfWriter lout, rout, blout, brout;
	int lcount = 0, rcount = 0, blcount = 0, brcount = 0;


	string fileName = "data.txt";
	ofstream txtOut;
	txtOut.open(fileName);
		

#pragma omp parallel sections num_threads(8)
			{
#pragma omp section
				{
					ImagePtr img;
					Mat frame;
					bool firstframe = true;

					while (true)
					{
						if (lq.try_dequeue(img))
						{


							
							lfps = ConvertTimeToFPS(img->GetTimeStamp(), llast); // TimeStamp is in nanoseconds
							llast = img->GetTimeStamp();
							
							unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
							Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);

							if (firstframe)
							{
								lbg = tframe.clone();
								firstframe = false;
							}


							/*
							frame = tframe.clone();


							//putText(frame, to_string(lfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							putText(frame, "theta: " + to_string(theta) + " phi: " + to_string(phi) + ' ' + to_string(lfps) + " Hz ", Point((imageWidth - 400), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							putText(frame, to_string(pt3dtemp.at<double>(0, 0)) + ' ' + to_string(pt3dtemp.at<double>(1, 0)) + ' ' + to_string(pt3dtemp.at<double>(2, 0)), Point((imageWidth - 400), 30), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							putText(frame, to_string(initX) + ' ' + to_string(initY) + ' ' + to_string(vol_x) + ' ' + to_string(vol_y), Point((imageWidth - 400), 45), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							
							// if you see complete black, maybe the exposure time is too low
							ldisp_frame.try_enqueue(frame.clone());
							*/


							if (isLight)
								txtOut << theta << ' ' << phi << ' ' << lfps << ' ' << pt3dtemp.at<double>(0, 0) << ' ' << pt3dtemp.at<double>(1, 0) << ' ' << pt3dtemp.at<double>(2, 0) << ' ' << vol_x << ' ' << vol_y << endl;



							lmask.try_enqueue(tframe.clone());







						}

						if (!stream)
							break;
					}
				}
#pragma omp section
				{
					ImagePtr img;
					Mat frame;
					bool firstframe = true;
					while (true)
					{
						if (rq.try_dequeue(img))
						{
							/*
							rfps = ConvertTimeToFPS(img->GetTimeStamp(), rlast); // TimeStamp is in nanoseconds
							rlast = img->GetTimeStamp();
							*/
							unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
							Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);

							if (firstframe)
							{
								rbg = tframe.clone();
								firstframe = false;
							}


							/*
							frame = tframe.clone();
							putText(frame, to_string(rfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							*/



							rmask.try_enqueue(tframe.clone());
							// rdisp_frame.try_enqueue(frame.clone());
						}
						if (!stream)
							break;
					}
				}






#pragma omp section
				{
					Mat frame, mask;
					Mat tmask;
					Mat temp_mask;
					double min, max;
					Point min_loc, max_loc;

					while (true)
					{
						if (lmask.try_dequeue(frame))
						{

							subtract(frame, lbg, tmask);
							//threshold(tmask, tmask, 1, 255, CV_THRESH_BINARY);

							//putText(mask, to_string(ltrk_frame.size_approx()), Point((imageWidth - 50), 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							//ldisp_mask.try_enqueue(mask.clone());


							
							minMaxLoc(tmask, &min, &max, &min_loc, &max_loc);

							if (max > 50)
								isLight = true;
							else
								isLight = false;
							/*
							temp_mask = tmask.clone();
							if (max > 50)
								putText(temp_mask, "Light " + to_string(max_loc.x) + ' ' + to_string(max_loc.y), Point((imageWidth - 300), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							else
								putText(temp_mask, "No Light " + to_string(max), Point((imageWidth - 200), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							ldisp_frame.try_enqueue(temp_mask.clone());
							*/
							lptq.try_enqueue(max_loc);
						}

						if (!stream)
							break;
					}
				}



#pragma omp section
				{
					Mat frame, mask;
					Mat tmask;
					Mat temp_mask;
					double min, max;
					Point min_loc, max_loc;

					while (true)
					{
						if (rmask.try_dequeue(frame))
						{

							subtract(frame, rbg, tmask);
							//threshold(tmask, tmask, 1, 255, CV_THRESH_BINARY);

							//putText(mask, to_string(ltrk_frame.size_approx()), Point((imageWidth - 50), 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							//ldisp_mask.try_enqueue(mask.clone());



							minMaxLoc(tmask, &min, &max, &min_loc, &max_loc);


							/*
							temp_mask = tmask.clone();
							if (max > 50)
								putText(temp_mask, "Light " + to_string(max_loc.x) + ' ' + to_string(max_loc.y), Point((imageWidth - 300), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							else
								putText(temp_mask, "No Light " + to_string(max), Point((imageWidth - 200), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							rdisp_frame.try_enqueue(temp_mask.clone());
							*/
							rptq.try_enqueue(max_loc);

							
						}

						if (!stream)
							break;
					}
				}







// trial do one pair and direct transfer to conversion thread.

// do two pairs of cameras only each go to a queue, then another take both and take the average and put into another queue (do not do more)

		// l_camera r_camera center of region of interest -> catesian 3D
#pragma omp section
				{

					Point point1, point2;
					Mat extPoint1(3, 1, CV_64F);
					Mat extPoint2(3, 1, CV_64F);
					// Mat pt3dtemp(3, 1, CV_64F);
					while (true)
					{

						while (!lptq.try_dequeue(point1))
						{
							if (!stream)
								break;
						}
						while (!rptq.try_dequeue(point2))
						{
							if (!stream)
								break;
						}

						extPoint1.at<double>(0, 0) = point1.x;
						extPoint1.at<double>(1, 0) = point1.y;
						extPoint1.at<double>(2, 0) = 1;

						extPoint2.at<double>(0, 0) = point2.x;
						extPoint2.at<double>(1, 0) = point2.y;
						extPoint2.at<double>(2, 0) = 1;


						if ((point1.x != -1 && point1.y != -1) || (point2.x != -1 && point2.y != -1))
						{

							pt3dtemp = triangulate_Linear_LS(camMat[0] * RTMat[0], camMat[1] * RTMat[1], extPoint1, extPoint2);
							signalq.try_enqueue(pt3dtemp);		
						}



						/*
						else
						{
							
							pt3dtemp.at<double>(0, 0) = -1;
							pt3dtemp.at<double>(1, 0) = -1;
							pt3dtemp.at<double>(2, 0) = -1;
							
						}
						*/

					}
				}




#pragma omp section
				{

					Point3f obj_pos;


					Point2f galvo_pos;
					// potential x,y offset
					galvo_pos.x = 0;
					galvo_pos.y = 0;
					// Lx, Ly
					double galvo_height_theta = 96.1516;
					// double galvo_height_phi = 94.28662;
					double galvo_height_phi = galvo_height_theta - 1.5;
					double multiplier = 2;

					

					
					galvo_signal(initX, initY);
					
					while (true)
					{


						if (signalq.try_dequeue(pt3dtemp))
						{ 
						
							/*
							ifstream in("obj_pos.txt");
							in >> obj_pos.x >> obj_pos.y >> obj_pos.z;
							*/

							// divide 2 for real theta -> galvo theta

							obj_pos.x = -pt3dtemp.at<double>(1, 0);
							obj_pos.y = -pt3dtemp.at<double>(0, 0);
							obj_pos.z = -pt3dtemp.at<double>(2, 0);
							try
							{

								/*
								theta = atan((pt3dtemp.at<double>(0, 0) - galvo_pos.x) / (galvo_height_theta - pt3dtemp.at<double>(2, 0))) * 180 / PI / 2;
								phi = atan((pt3dtemp.at<double>(1, 0) - galvo_pos.y) / (galvo_height_phi - pt3dtemp.at<double>(2, 0))) * 180 / PI / 2;
								*/



								theta = atan((obj_pos.x - galvo_pos.x) / (galvo_height_theta - obj_pos.z)) * 180 / PI / 2;
								phi = atan((obj_pos.y - galvo_pos.y) / (galvo_height_phi - obj_pos.z)) * 180 / PI / 2;


								// fake coordinate
								// theta = 2 * atan((0.16 - galvo_pos.x) / (galvo_height_theta - 0)) * 180 / PI;
								// phi = 2 * atan((0.16 - galvo_pos.y) / (galvo_height_phi - 0)) * 180 / PI;

								// int initX = 34290;
								// int initY = 29835;
								// check before activate signal
								if ((abs(theta) < 5) && (abs(phi) < 5))
								{

									// when you look at it, negative vol_x means positive theta, the other consistent

									/*
									vol_x = (int)(-theta / 20 * 65535 / multiplier);
									vol_y = (int)(phi / 20 * 65535 / multiplier);

									// add offset (initX always first coor, initY always 2nd coor)
									galvo_signal(vol_y + initX, vol_x + initY);

									*/

									vol_x = (int)(-theta / 20 * 65535 / multiplier);
									vol_y = (int)(phi / 20 * 65535 / multiplier);

									// add offset (initX always first coor, initY always 2nd coor)
									galvo_signal(vol_x + initX, vol_y + initY);

								}
							}
							catch (int e)
							{

							}

						}



					}
				}










				/*

// build and test incrementally, one thing at a time
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

*/

		}





	/*
	if (lout.IsOpen())
		lout.Close();
	if (rout.IsOpen())
		rout.Close();
	if (blout.IsOpen())
		blout.Close();
	if (brout.IsOpen())
		brout.Close();
	*/

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

		delete imageEventHandler0;
		delete imageEventHandler1;
		/*
		pCam = camList.GetByIndex(2);
		pCam->UnregisterEvent(*imageEventHandler2);
		pCam = camList.GetByIndex(3);
		pCam->UnregisterEvent(*imageEventHandler3);

		delete imageEventHandler2;
		delete imageEventHandler3;

		*/


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





    // close DAQ (for galvo) at the end
    S826_SystemClose();
	

    return 0;
}