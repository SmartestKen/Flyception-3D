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
				ImagePtr img;
				Mat frame;
				bool firstframe = true;
				while (true)
				{
					if (blq.try_dequeue(img))
					{
						blfps = ConvertTimeToFPS(img->GetTimeStamp(), bllast); // TimeStamp is in nanoseconds
						bllast = img->GetTimeStamp();
						unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
						Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);


						if (firstframe)
						{
							blbg = tframe.clone();
							firstframe = false;
						}


						frame = tframe.clone();
						putText(frame, to_string(blfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));

						blmask.try_enqueue(tframe.clone());
						// bldisp_frame.try_enqueue(frame.clone());
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
						if (brq.try_dequeue(img))
						{
							brfps = ConvertTimeToFPS(img->GetTimeStamp(), brlast); // TimeStamp is in nanoseconds
							brlast = img->GetTimeStamp();
							unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
							Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);


							if (firstframe)
							{
								brbg = tframe.clone();
								firstframe = false;
							}

							frame = tframe.clone();
							putText(frame, to_string(brfps) + " Hz", Point((imageWidth - 50), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							//putText(frame, to_string(lq.size_approx()), Point((imageWidth - 50), 25), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));


							brmask.try_enqueue(tframe.clone());
							// brdisp_frame.try_enqueue(frame.clone());
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
					if (blmask.try_dequeue(frame))
					{

						subtract(frame, blbg, tmask);
						//threshold(tmask, tmask, 1, 255, CV_THRESH_BINARY);

						//putText(mask, to_string(ltrk_frame.size_approx()), Point((imageWidth - 50), 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						//ldisp_mask.try_enqueue(mask.clone());



						minMaxLoc(tmask, &min, &max, &min_loc, &max_loc);

						temp_mask = tmask.clone();
						if (max > 50)
							putText(temp_mask, "Light " + to_string(max_loc.x) + ' ' + to_string(max_loc.y), Point((imageWidth - 300), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						else
							putText(temp_mask, "No Light " + to_string(max), Point((imageWidth - 200), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
						bldisp_frame.try_enqueue(temp_mask.clone());
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
						if (brmask.try_dequeue(frame))
						{

							subtract(frame, brbg, tmask);
							//threshold(tmask, tmask, 1, 255, CV_THRESH_BINARY);

							//putText(mask, to_string(ltrk_frame.size_approx()), Point((imageWidth - 50), 10), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							//ldisp_mask.try_enqueue(mask.clone());



							minMaxLoc(tmask, &min, &max, &min_loc, &max_loc);

							temp_mask = tmask.clone();
							if (max > 50)
								putText(temp_mask, "Light " + to_string(max_loc.x) + ' ' + to_string(max_loc.y), Point((imageWidth - 300), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							else
								putText(temp_mask, "No Light " + to_string(max), Point((imageWidth - 200), 15), FONT_HERSHEY_COMPLEX, 0.4, Scalar(255, 255, 255));
							brdisp_frame.try_enqueue(temp_mask.clone());
						}

						if (!stream)
							break;
					}
				}





//--------------------------------above is new temp----------------------------------------------------
// r_camera image -> foreground
#pragma omp section
		{
		ImagePtr img;
		Mat tmask;
		bool firstframe = true;
		Mat rbg;
		while (true)
		{
			if (rq.try_dequeue(img))
			{

				unsigned int rowBytes = img->GetStride();
				Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);



				if (firstframe)
				{
					rbg = tframe.clone();
					firstframe = false;
				}

				subtract(rbg, tframe, tmask);
				// threshold(tmask, tmask, 1, 255, cv::THRESH_BINARY);
				threshold(tmask, tmask, 1, 255, cv::THRESH_TOZERO);
				maskQ[1].enqueue(tmask);




			}
			if (!stream)
				break;
		}
		}





		// bl_camera image -> foreground
#pragma omp section
		{
			ImagePtr img;
			Mat tmask;
			bool firstframe = true;
			Mat blbg;
			while (true)
			{
				if (blq.try_dequeue(img))
				{
					unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
					Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);


					if (firstframe)
					{
						blbg = tframe.clone();
						firstframe = false;
					}

					subtract(blbg, tframe, tmask);

					// threshold(tmask, tmask, 1, 255, cv::THRESH_BINARY);
					threshold(tmask, tmask, 1, 255, cv::THRESH_TOZERO);
					maskQ[2].enqueue(tmask);




				}
				if (!stream)
					break;
			}
		}


		// br_camera image -> foreground
#pragma omp section
		{
			ImagePtr img;
			Mat tmask;
			bool firstframe = true;
			Mat brbg;
			while (true)
			{
				if (brq.try_dequeue(img))
				{
					unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
					Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);



					if (firstframe)
					{
						brbg = tframe.clone();
						firstframe = false;
					}

					subtract(brbg, tframe, tmask);
					// threshold(tmask, tmask, 1, 255, cv::THRESH_BINARY);
					threshold(tmask, tmask, 1, 255, cv::THRESH_TOZERO);

					maskQ[3].enqueue(tmask);




				}
				if (!stream)
					break;
			}
		}



		// l_camera foreground -> center of region of interest
#pragma omp section
		{
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));
			int mask_count = 0;
			Mat mask;

			while (true)
			{
				Point2f pt(-1, -1);

				if (maskQ[0].try_dequeue(mask))
				{


					double min, max;
					Point min_loc, max_loc;
					minMaxLoc(mask, &min, &max, &min_loc, &max_loc);
					if ((mask_count % 50) == 0)
					{
						cout << min << ' ' << max << endl;
						mask_count = 0;
					}
					mask_count++;
					pointQ[0].enqueue(max_loc);
					pointQ_Copy[0].enqueue(max_loc);

					/*
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
							// drawContours(mask, contours, j, Scalar(255, 255, 255), CV_FILLED);

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
					pointQ[0].enqueue(pt);
					pointQ_Copy[0].enqueue(pt);
					*/
				}

				if (!stream)
					break;
			}
		}



		// r_camera foreground -> center of region of interest
#pragma omp section
		{
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));

			Mat mask;

			while (true)
			{
				Point2f pt(-1, -1);

				if (maskQ[1].try_dequeue(mask))
				{


					double min, max;
					Point min_loc, max_loc;
					minMaxLoc(mask, &min, &max, &min_loc, &max_loc);
					pointQ[1].enqueue(max_loc);
					pointQ_Copy[1].enqueue(max_loc);

					/*
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
							// drawContours(mask, contours, j, Scalar(255, 255, 255), CV_FILLED);

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
					pointQ[1].enqueue(pt);
					pointQ_Copy[1].enqueue(pt);
					*/



				}

				if (!stream)
					break;
			}
		}

		// bl_camera foreground -> center of region of interest
#pragma omp section
		{
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));

			Mat mask;

			while (true)
			{
				Point2f pt(-1, -1);

				if (maskQ[2].try_dequeue(mask))
				{


					double min, max;
					Point min_loc, max_loc;
					minMaxLoc(mask, &min, &max, &min_loc, &max_loc);
					pointQ[2].enqueue(max_loc);
					pointQ_Copy[2].enqueue(max_loc);

					/*
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
							// drawContours(mask, contours, j, Scalar(255, 255, 255), CV_FILLED);

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
					pointQ[2].enqueue(pt);
					pointQ_Copy[2].enqueue(pt);
					*/
				}

				if (!stream)
					break;
			}
		}


		// br_camera foreground -> center of region of interest
#pragma omp section
		{
			Mat element = getStructuringElement(MORPH_RECT, Size(3, 3), Point(1, 1));

			Mat mask;

			while (true)
			{
				Point2f pt(-1, -1);

				if (maskQ[3].try_dequeue(mask))
				{


					double min, max;
					Point min_loc, max_loc;
					minMaxLoc(mask, &min, &max, &min_loc, &max_loc);
					pointQ[3].enqueue(max_loc);
					pointQ_Copy[3].enqueue(max_loc);



					/*
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
							drawContours(mask, contours, j, Scalar(255, 255, 255), cv::FILLED);

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
					pointQ[3].enqueue(pt);
					pointQ_Copy[3].enqueue(pt);
					*/
				}

				if (!stream)
					break;
			}
		}


		// l_camera r_camera center of region of interest -> catesian 3D
#pragma omp section
		{
			int idx0 = 0;
			int idx1 = 1;

			Point point1, point2;
			Mat extPoint1(3, 1, CV_64F);
			Mat extPoint2(3, 1, CV_64F);
			Mat pt3dtemp(3, 1, CV_64F);
			while (true)
			{

				while (!pointQ[idx0].try_dequeue(point1))
				{
					if (!stream)
						break;
				}
				while (!pointQ_Copy[idx1].try_dequeue(point2))
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

					pt3dtemp = triangulate_Linear_LS(camMat[idx0] * RTMat[idx0], camMat[idx1] * RTMat[idx1], extPoint1, extPoint2);
				}
				else
				{
					pt3dtemp.at<double>(0, 0) = -1;
					pt3dtemp.at<double>(1, 0) = -1;
					pt3dtemp.at<double>(2, 0) = -1;
				}

				pt3dQ[idx0].enqueue(pt3dtemp);


			}
		}

		// r_camera bl_camera center of region of interest -> catesian 3D
#pragma omp section
		{
			int idx0 = 1;
			int idx1 = 2;

			Point point1, point2;
			Mat extPoint1(3, 1, CV_64F);
			Mat extPoint2(3, 1, CV_64F);
			Mat pt3dtemp(3, 1, CV_64F);
			while (true)
			{

				while (!pointQ[idx0].try_dequeue(point1))
				{
					if (!stream)
						break;
				}
				while (!pointQ_Copy[idx1].try_dequeue(point2))
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

					pt3dtemp = triangulate_Linear_LS(camMat[idx0] * RTMat[idx0], camMat[idx1] * RTMat[idx1], extPoint1, extPoint2);
				}
				else
				{
					pt3dtemp.at<double>(0, 0) = -1;
					pt3dtemp.at<double>(1, 0) = -1;
					pt3dtemp.at<double>(2, 0) = -1;
				}

				pt3dQ[idx0].enqueue(pt3dtemp);


			}
		}

		// bl_camera br_camera center of region of interest -> catesian 3D
#pragma omp section
		{
			int idx0 = 2;
			int idx1 = 3;

			Point point1, point2;
			Mat extPoint1(3, 1, CV_64F);
			Mat extPoint2(3, 1, CV_64F);
			Mat pt3dtemp(3, 1, CV_64F);
			while (true)
			{

				while (!pointQ[idx0].try_dequeue(point1))
				{
					if (!stream)
						break;
				}
				while (!pointQ_Copy[idx1].try_dequeue(point2))
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

					pt3dtemp = triangulate_Linear_LS(camMat[idx0] * RTMat[idx0], camMat[idx1] * RTMat[idx1], extPoint1, extPoint2);
				}
				else
				{
					pt3dtemp.at<double>(0, 0) = -1;
					pt3dtemp.at<double>(1, 0) = -1;
					pt3dtemp.at<double>(2, 0) = -1;
				}

				pt3dQ[idx0].enqueue(pt3dtemp);


			}
		}

		// br_camera l_camera center of region of interest -> catesian 3D
#pragma omp section
		{
			int idx0 = 3;
			int idx1 = 0;

			Point point1, point2;
			Mat extPoint1(3, 1, CV_64F);
			Mat extPoint2(3, 1, CV_64F);
			Mat pt3dtemp(3, 1, CV_64F);
			while (true)
			{

				while (!pointQ[idx0].try_dequeue(point1))
				{
					if (!stream)
						break;
				}
				while (!pointQ_Copy[idx1].try_dequeue(point2))
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

					pt3dtemp = triangulate_Linear_LS(camMat[idx0] * RTMat[idx0], camMat[idx1] * RTMat[idx1], extPoint1, extPoint2);
				}
				else
				{
					pt3dtemp.at<double>(0, 0) = -1;
					pt3dtemp.at<double>(1, 0) = -1;
					pt3dtemp.at<double>(2, 0) = -1;
				}

				pt3dQ[idx0].enqueue(pt3dtemp);


			}
		}


		// final 3D coordinate and conversion
#pragma omp section
		{

			Mat pt3dtemp(3, 1, CV_64F);
			vector<vector<double>> coordTemp(3, vector<double>(4, -1));
			Mat pt3d(3, 1, CV_64F);
			while (true)
			{
				/*
				for (int i = 0; i < 4; i++)
				{
					while (!pt3dQ[i].try_dequeue(pt3dtemp))
					{
						if (!stream)
							break;
					}
					for (int k = 0; k < 3; k++)
						coordTemp[k][i] = pt3dtemp.at<double>(k, 0);


				}

				for (int k = 0; k < 3; k++)
				{
					vector<double> curCoord = coordTemp[k];
					const auto median_it = curCoord.begin() + curCoord.size() / 2;
					nth_element(curCoord.begin(), median_it, curCoord.end());
					pt3d.at<double>(k, 0) = *median_it;
				}
				//cout << pt3d.at<double>(0, 0) << ' ' << pt3d.at<double>(1, 0) << ' ' << pt3d.at<double>(2, 0) << endl;
				// printf("%.2f %.2f %.2f\r", pt3d.at<double>(0, 0), pt3d.at<double>(1, 0), pt3d.at<double>(2, 0));

				if (!stream)
					break;
				*/

				// cout << lq.size_approx() << ' ' << maskQ[0].size_approx() << ' ' << pointQ[0].size_approx() << endl;


				Point2f galvo_pos;

				// Lx, Ly
				double galvo_height_theta = 2;
				double galvo_height_phi = 2;
				Point3f galvo_pos_phi;
				Point3f obj_pos;
				Point2f diff_pos;
				double height_diff_theta;
				double height_diff_phi;


				galvo_pos.x = 0;
				galvo_pos.y = 0;

				double theta;
				double phi;

				ifstream in("obj_pos.txt");

				in >> obj_pos.x >> obj_pos.y >> obj_pos.z;


				diff_pos.x = obj_pos.x - galvo_pos.x;
				diff_pos.y = obj_pos.y - galvo_pos.y;
				height_diff_theta = obj_pos.z - galvo_height_theta;
				height_diff_phi = obj_pos.z - galvo_height_phi;

				theta = 2 * atan(diff_pos.x / height_diff_theta);
				phi = 2 * atan(diff_pos.y / height_diff_phi);

				galvo_signal((int)theta / 10 * 32768 + 32768, (int)phi / 10 * 32768 + 32768);
				//cout << theta << ' ' << phi << endl;




			}






		}







		/*
		// useless inner loop triangulation version
#pragma omp section
		{
			Point points[4];
			vector<Mat> extPoints(4);
			for (int i = 0; i < 4; i++)
				extPoints[i].push_back(Mat(3, 1, CV_64F));
			Mat pt3dtemp(3, 1, CV_64F);
			vector<vector<double>> coordTemp(3, vector<double>(4, -1));
			Mat pt3d(3, 1, CV_64F);

			while (true)
			{
				for (int i = 0; i < 4; i++)
				{
					while (!pointQ[i].try_dequeue(points[i]))
					{
						if (!stream)
							break;
					}
					extPoints[i].at<double>(0, 0) = points[i].x;
					extPoints[i].at<double>(1, 0) = points[i].y;
					extPoints[i].at<double>(2, 0) = 1;
				}


				for (int i = 0; i < 4; i++)
				{
					// if i is at last camera, j becomes the first camera (circular)
					int j = (i == 3) ? 0 : (i + 1);
						// we use ith and (i+1)th cameras to triangulate
					if ((points[i].x != -1 && points[i].y != -1) || (points[j].x != -1 && points[j].y != -1))
					{

						pt3dtemp = triangulate_Linear_LS(camMat[i] * RTMat[i], camMat[j] * RTMat[j], extPoints[i], extPoints[j]);
					}
					else
					{
						pt3dtemp.at<double>(0, 0) = -1;
						pt3dtemp.at<double>(1, 0) = -1;
						pt3dtemp.at<double>(2, 0) = -1;
					}

					// collect coordinate wise for median
					for (int k = 0; k < 3; k++)
						coordTemp[k][i] = pt3dtemp.at<double>(k, 0);
				}

				for (int k = 0; k < 3; k++)
				{
					vector<double> curCoord = coordTemp[k];
					const auto median_it = curCoord.begin() + curCoord.size() / 2;
					nth_element(curCoord.begin(), median_it, curCoord.end());
					pt3d.at<double>(k, 0) = *median_it;
				}
				cout << pt3d.at<double>(0, 0) << ' ' << pt3d.at<double>(1, 0) << ' ' << pt3d.at<double>(2, 0) << endl;
				if (!stream)
					break;
			}
		}



		*/






















































		/*
		#pragma omp section
		{
			ImagePtr tImage;
			int count = 0;
			while (true)
			{
				//std::cout << "lrec size: " << lrec.size_approx() << endl;
				if (lrec.try_dequeue(tImage))
				{
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/source/lout_%d.jpg", count);
					tImage->Save(buffer, JPEG);
					count++;

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
			int count = 0;
			while (true)
			{
				if (rrec.try_dequeue(tImage))
				{
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/source/rout_%d.jpg", count);
					tImage->Save(buffer, JPEG);
					count++;

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
			int count = 0;
			while (true)
			{
				if (blrec.try_dequeue(tImage))
				{
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/source/blout_%d.jpg", count);
					tImage->Save(buffer, JPEG);
					count++;


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
			int count = 0;
			while (true)
			{
				if (brrec.try_dequeue(tImage))
				{
					char buffer[100];

					sprintf_s(buffer, "C:/Users/Administrator/source/brout_%d.jpg", count);
					tImage->Save(buffer, JPEG);
					count++;
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
		*/
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