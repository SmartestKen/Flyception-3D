

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














































#pragma omp parallel sections num_threads(1)
	{

	// l_camera image -> foreground
#pragma omp section
		{

			ImagePtr img;
			Mat tmask;
			bool firstframe = true;
			Mat lbg;

			while (true)
			{
				if (lq.try_dequeue(img))
				{
					unsigned int rowBytes = img->GetStride(); //(double)img.GetReceivedDataSize() / (double)img.GetRows();
					Mat tframe = Mat(img->GetHeight(), img->GetWidth(), CV_8UC1, img->GetData(), rowBytes);


					if (firstframe)
					{
						lbg = tframe.clone();
						firstframe = false;
					}

					cv::subtract(lbg, tframe, tmask);

					double min, max;
					Point min_loc, max_loc;
					minMaxLoc(tmask, &min, &max, &min_loc, &max_loc);

					cout << max << endl;
					// threshold(tmask, tmask, 1, 255, cv::THRESH_BINARY);
					// cv::threshold(tmask, tmask, 1, 255, cv::THRESH_TOZERO);
					// maskQ[0].enqueue(tmask);










				}

				if (!stream)
					break;
			}
		}

	}