#pragma once

#include <SDKDDKVer.h>

#include <stdio.h>
#include <tchar.h>

#include <iostream>
#include <iomanip>
#include <string>

#include <opencv2/core/core.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgcodecs.hpp>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/video.hpp>


using namespace std;
using namespace cv;


// height from base needed when chessboard has a thickness
#define HEIGHT_FROM_BASE 0.0

std::vector<cv::Point3f> create3DChessboardCorners(cv::Size boardSize, float squareSize, int corner1, int corner2, int corner3, int corner4)
{
	// 0 and 3 are opposite, 1 and 2 are opposite (i.e. has to appear both at start/end)
	vector<pair<int, int>> corner_loc = { make_pair(-3,-4), make_pair(3,-4), make_pair(-3,4), make_pair(3,4) };

	// create the 3D points of your chessboard in its own coordinate system
	std::vector<cv::Point3f> corners;

	int i;
	int j;
	int total_count = 0;
	if (corner_loc[corner1].first == corner_loc[corner2].first)
	{ 
		i = corner_loc[corner1].second;
		j = corner_loc[corner1].first;

		int diff_i = corner_loc[corner4].second - corner_loc[corner1].second;
		int diff_j = corner_loc[corner4].first - corner_loc[corner1].first;


		while (abs(i) <= abs(corner_loc[corner4].second))
		{
			while (abs(j) <= abs(corner_loc[corner4].first))
			{
				cout << i << ' ' << j << endl;
				corners.push_back(cv::Point3f(float(i * squareSize), float(j * squareSize), HEIGHT_FROM_BASE));
				total_count += 1;
				// add the sign function (1 or -1)
				
				j += (diff_j > 0) - (diff_j < 0);
			}
			i += (diff_i > 0) - (diff_i < 0);
			j = corner_loc[corner1].first;
		}


	}
	else
	{
		i = corner_loc[corner1].first;
		j = corner_loc[corner1].second;

		int diff_i = corner_loc[corner4].first - corner_loc[corner1].first;
		int diff_j = corner_loc[corner4].second - corner_loc[corner1].second;

		while (abs(i) <= abs(corner_loc[corner4].first))
		{
			while (abs(j) <= abs(corner_loc[corner4].second))
			{
				cout << i << ' ' << j << endl;
				corners.push_back(cv::Point3f(float(i * squareSize), float(j * squareSize), HEIGHT_FROM_BASE));
				total_count += 1;
				// add the sign function (1 or -1)

				j += (diff_j > 0) - (diff_j < 0);
			}
			i += (diff_i > 0) - (diff_i < 0);
			j = corner_loc[corner1].second;
		}


	}




	cout << total_count << endl;
	return corners;
}

int defOrigin(int idx)
{
	/*
	string infilename = "..\\arena\\out_camera_data.xml";
	string outfilename = "..\\arena\\camera_projection_data.xml";
	string imgfile = "..\\arena\\cbview_09192016.jpg";
	*/

	std::string intrinsicParaFile = "intrinsic_para_" + std::to_string(idx + 1) + ".xml";
	std::string extrinsicParaFile = "extrinsic_para_" + std::to_string(idx + 1) + ".xml";
	std::string imgFile = "origin_ref_" + std::to_string(idx + 1) + ".jpg";
	std::string cornerFile = "corner_config_" + std::to_string(idx + 1) + ".txt";



	int boardHeight, boardWidth;
	float squareSize;

	Mat img = imread(imgFile);

	Mat cameraMatrix, distCoeffs;

	std::vector<cv::Point2f> imagePoints;
	std::vector<cv::Point3f> objectPoints;

	cv::Mat rvec(1, 3, cv::DataType<double>::type);
	cv::Mat tvec(1, 3, cv::DataType<double>::type);
	cv::Mat rotationMatrix(3, 3, cv::DataType<double>::type);

	FileStorage fs(intrinsicParaFile, FileStorage::READ);

	fs["camera_matrix"] >> cameraMatrix;
	fs["distortion_coefficients"] >> distCoeffs;
	fs["board_height"] >> boardHeight;
	fs["board_width"] >> boardWidth;
	fs["square_size"] >> squareSize;

	fs.release();

	Size cbSize = Size(boardWidth, boardHeight);

	bool found = findChessboardCorners(img, cbSize, imagePoints);

	if (found)
	{
		// red is the start and follow along the drawing line
		drawChessboardCorners(img, cbSize, imagePoints, found);
		// putText(img, to_string(idx), 3, 0.4, Scalar(255, 255, 255));

		putText(img, "cam " + to_string(idx+1), Point((1080 - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
		imshow("image", img);
		// imshow must come with waitKey
		waitKey(0);

		int corner1 = -1;
		int corner2 = -1;
		int corner3 = -1;
		int corner4 = -1;

		std::ifstream in(cornerFile);
		in >> corner1 >> corner2 >> corner3 >> corner4;
		cout << corner1 << endl;


		objectPoints = create3DChessboardCorners(cbSize, squareSize, corner1, corner2, corner3, corner4);

		cv::solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec);
		cv::Rodrigues(rvec, rotationMatrix);

		//show the pose estimation data
		cout << fixed << setprecision(2) << "rvec = ["
			<< rvec.at<double>(0, 0) << ", "
			<< rvec.at<double>(1, 0) << ", "
			<< rvec.at<double>(2, 0) << "] \t" << "tvec = ["
			<< tvec.at<double>(0, 0) << ", "
			<< tvec.at<double>(1, 0) << ", "
			<< tvec.at<double>(2, 0) << "]" << endl;




		// cv::Mat tvec_tp = tvec.t();
		cv::Mat rtMat(3, 4, cv::DataType<double>::type);
		hconcat(rotationMatrix, tvec, rtMat);

		FileStorage fs(extrinsicParaFile, FileStorage::WRITE);


		fs << "rvec" << rvec;
		fs << "tvec" << tvec;
		fs << "rotation_matrix" << rotationMatrix;
		fs << "rtMat" << rtMat;

		fs.release();
	}
	else
	{
		cout << "could not find";
		imshow("image", img);
	}


	return 0;
}

