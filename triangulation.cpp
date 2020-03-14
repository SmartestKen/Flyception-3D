// fmfrecord.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Spinnaker.h"
#include "SpinGenApi/SpinnakerGenApi.h"
#include <iostream>
#include <sstream>


using namespace Spinnaker;
using namespace Spinnaker::GenApi;
using namespace Spinnaker::GenICam;
using namespace std;
using namespace cv;
using namespace moodycamel;



bool stream = true;


int imageWidth = 1080, imageHeight = 1080;

// relates to grabbing function, to be modified
int llast = 0, lfps = 0;
int rlast = 0, rfps = 0;


#define camCount 4;

Mat camMat[camCount];
Mat RTMat[camCount];
CameraPtr cameras[camCount];

// images from cameras
ReaderWriterQueue<Image> imageQ[camCount];
ReaderWriterQueue<Image> maskQ[camCount];
ReaderWriterQueue<Point> pointQ[camCount];



int ConvertTimeToFPS(int ctime, int ltime)
{
	int dtime;

	if (ctime < ltime)
		dtime = ctime + (8000 - ltime);
	else
		dtime = ctime - ltime;

	if (dtime > 0)
		dtime = 8000 / dtime;
	else
		dtime = 0;

	return dtime;
}

void OnLeftImageGrabbed(Image* pImage, const void* pCallbackData)
{
	Image img;

	lfps = ConvertTimeToFPS(pImage->GetTimeStamp().cycleCount, llast);
	llast = pImage->GetTimeStamp().cycleCount;

	img.DeepCopy(pImage);

	if (stream)
		lq.enqueue(img);

	return;
}

void OnRightImageGrabbed(Image* pImage, const void* pCallbackData)
{
	Image img;

	rfps = ConvertTimeToFPS(pImage->GetTimeStamp().cycleCount, rlast);
	rlast = pImage->GetTimeStamp().cycleCount;

	img.DeepCopy(pImage);

	if (stream)
		rq.enqueue(img);

	return;
}

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

int main()
{


	// Retrieve singleton reference to system object
	SystemPtr system = System::GetInstance();
	// Retrieve list of cameras from the system
	CameraList camList = system->GetCameras();


	// ensure the size is consistent with parameter arrays
	// if consistent, read in camera list and data
	if (camCount != camList.GetSize())
	{
		cerr << "Actual camera count != parameters count" << endl;
		// Clear camera list before releasing system
		camList.Clear();
		// Release system
		system->ReleaseInstance();
		return -1;
	}
	else
	{
		// stackoverflow.com/questions/16295551/how-to-correctly-use-cvtriangulatepoints
		// www.mathworks.com/help/vision/ug/camera-calibration.html
		// camMat intrinsic 3x3 mat
		// RTMat rotation translation 3x4 mat

		for (int i = 0; i < camCount; i++) {
			FileStorage fs_in("intrinsic_para_"+to_string(i+1)+".xml", FileStorage::READ);
			FileStorage fs_ex("extrinsic_para_" + to_string(i+1) + ".xml", FileStorage::READ);

			fs_in["camera_matrix"] >> camMat[i];
			fs_ex["rtMat"] >> RTMat[i];


			fs_in.release();
			fs_ex.release();
		}
		
	}



	/*
	// set cam parameters (merge to the previous loop)
	lcam.SetCameraParameters(imageWidth, imageHeight);
	lcam.SetProperty(SHUTTER, 1.998);
	lcam.SetProperty(GAIN, 0.0);
	lcam.SetTrigger();
	lcam.cam.StartCapture(OnLeftImageGrabbed);
	*/




	// take background image separately (as file or before confirm)
	Mat lbg, rbg;

	stream = true;


#pragma omp parallel sections num_threads(10)
	{
		// from image to foreground
#pragma omp section
		{
			Image img;
			Mat mask, tmask;

			while (true)
			{
				if (imageQ[i].try_dequeue(img))
				{
					unsigned int rowBytes = (double)img.GetReceivedDataSize() / (double)img.GetRows();
					Mat frame = Mat(img.GetRows(), img.GetCols(), CV_8UC1, img.GetData(), rowBytes);

					subtract(lbg, frame, tmask);
					threshold(tmask, tmask, 1, 255, CV_THRESH_BINARY);

					mask = tmask.clone();

					maskQ[i].enqueue(mask);
				}

				if (!stream)
					break;
			}
		}


		// from foreground to center of region of interest
#pragma omp section
		{
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));

			Mat mask;

			while (true)
			{
				Point2f pt(-1, -1);

				if (maskQ[i].try_dequeue(mask))
				{
					erode(mask, mask, element, Point(-1, -1), 3);
					dilate(mask, mask, element, Point(-1, -1), 3);

					vector<vector<Point>> contours;
					findContours(mask, contours, RETR_EXTERNAL, CHAIN_APPROX_SIMPLE);

					if (contours.size() > 0)
					{
						// Get the camera moments and mass centers
						vector<Moments> mu(contours.size());
						vector<Point2f> mc(contours.size());

						double max_size = 0;
						int k;

						for (int j = 0; j < contours.size(); j++)
						{
							drawContours(mask, contours, j, Scalar(255, 255, 255), CV_FILLED);

							mu[j] = moments(contours[j], false);
							mc[j] = Point2f(mu[j].m10 / mu[j].m00, mu[j].m01 / mu[j].m00);

							double csize = contourArea(contours[j]);

							if (csize > max_size)
							{
								k = j;
								max_size = csize;
							}
						}

						pt = mc[k];

					}
					pointQ[i].enqueue(pt);
				}

				if (!stream)
					break;
			}
		}


		// triangulation, center of region of interest -> 3d location
#pragma omp section
		{
			Point points[camCount];
			Mat extPoints(1, 1, CV_64FC3)[camCount];
			Mat pt3dtemp(1, 1, CV_64FC3);
			vector<vector<double>> coordTemp(3, vector<double>(camCount, -1));
			Mat pt3d(1, 1, CV_64FC3);

			while (true)
			{
				for (int i = 0; i < camCount; i++)
				{
					while (!pointQ[i].try_dequeue(points[i]))
					{
						if (!stream)
							break;
					}
					extPoints[i].at<double>(0, 0) = points[i].x
						extPoints[i].at<double>(1, 0) = points[i].y
						extPoints[i].at<double>(2, 0) = 1
				}


				for (int i = 0; i < camCount; i++)
				{
					// if i is at last camera, j becomes the first camera (circular)
					int j = (i == camCount - 1) ? 0 : (i + 1)
						// we use ith and (i+1)th cameras to triangulate
						if ((points[i].x != -1 && points[i].y != -1) || (points[j].x != -1 && points[j].y != -1))
							pt3dtemp = triangulate_Linear_LS(camMat[i] * RTMat[i], camMat[j] * RTMat[j], extPoints[i], extPoints[j]);
						else
						{
							pt3dtemp.at<double>(0, 0) = -1;
							pt3dtemp.at<double>(1, 0) = -1;
							pt3dtemp.at<double>(2, 0) = -1;
						}

					// collect coordinate wise for median
					for (int k = 0; k < 3; k++)
						coordTemp[k][i] = pt3dtemp.at<double>(k, 0)
				}

				for (int k = 0; k < 3; k++)
				{
					vector<double> curCoord = coordTemp[k]
						const auto median_it = curCoord.begin() + curCoord.size() / 2;
					nth_element(curCoord.begin(), median_it, curCoord.end());
					pt3d.at<double>(k, 0) = *median_it;
				}

				if (!stream)
					break;
			}
		}


		// key termination thread
#pragma omp section
		{
			while (true)
			{
				if (GetAsyncKeyState(VK_ESCAPE))
				{
					stream = false;
					break;
				}
			}
		}
	}

	camList.Clear();
	// Release system
	system->ReleaseInstance();

	return 0;
}

