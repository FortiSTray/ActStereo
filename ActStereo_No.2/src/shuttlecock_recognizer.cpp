#include "shuttlecock_recognizer.h"

ShuttleRecognizer::ShuttleRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight) :
	MVStereo(camNameL, argsLeft, camNameR, argsRight)
{
	bgModel = createBackgroundSubtractorMOG2(500, 16.0, false).dynamicCast<BackgroundSubtractor>();

	element = getStructuringElement(MORPH_RECT, Size(3, 3));

	cornerLeft = Mat::zeros(SRC_ROWS, SRC_COLS, CV_8UC1);
	cornerRight = Mat::zeros(SRC_ROWS, SRC_COLS, CV_8UC1);
}

void ShuttleRecognizer::backgroundSubtract()
{
	bgModel->apply(getFrameLeft(), fgMaskLeft, 0.005);
	bgModel->apply(getFrameRight(), fgMaskRight, 0.005);

	//Filter
	//GaussianBlur(fgMaskLeft, fgMaskLeft, Size(11, 11), 3.5, 3.5);
	threshold(fgMaskLeft, fgMaskLeft, 10, 255, THRESH_BINARY);
	//GaussianBlur(fgMaskRight, fgMaskRight, Size(11, 11), 3.5, 3.5);
	threshold(fgMaskRight, fgMaskRight, 10, 255, THRESH_BINARY);

	fgImageLeft = Scalar::all(0);
	getFrameLeft().copyTo(fgImageLeft, fgMaskLeft);
	fgImageRight = Scalar::all(0);
	getFrameRight().copyTo(fgImageRight, fgMaskRight);

	//imshow("LeftFg", fgImageLeft);
	//imshow("RightFg", fgImageRight);
}

void ShuttleRecognizer::preProc()
{
	//dilate(fgImageLeft, fgImageLeft, element, Point(-1, -1), 3);
	//erode(fgImageLeft, fgImageLeft, element, Point(-1, -1), 6);
	//dilate(fgImageLeft, fgImageLeft, element, Point(-1, -1), 3);

	//dilate(fgImageRight, fgImageRight, element, Point(-1, -1), 3);
	//erode(fgImageRight, fgImageRight, element, Point(-1, -1), 6);
	//dilate(fgImageRight, fgImageRight, element, Point(-1, -1), 3);

	medianBlur(fgImageLeft, fgImageLeft, 3);
	medianBlur(fgImageRight, fgImageRight, 3);

	cvtColor(fgImageLeft, dstLeft, CV_BGR2GRAY);
	cvtColor(fgImageRight, dstRight, CV_BGR2GRAY);

	threshold(dstLeft, dstLeft, 0, 255, THRESH_BINARY);
	threshold(dstRight, dstRight, 0, 255, THRESH_BINARY);
}

void ShuttleRecognizer::detectCorner(Mat &image, Mat &cornerImage, Size coreSize, int thresh)
{
	cornerImage = Mat::zeros(image.rows, image.cols, CV_8UC1);

	for (auto i = coreSize.height / 2; i < image.rows - coreSize.height / 2; i++)
		for (auto j = coreSize.width / 2; j < image.cols - coreSize.width / 2; j++)
		{
			int pixelCnt = 0;

			for (auto m = i - coreSize.height / 2; m <= i + coreSize.height / 2; m++)
				for (auto n = j - coreSize.width / 2; n <= j + coreSize.width / 2; n++)
				{
					if (image.ptr<uchar>(m)[n] != image.ptr<uchar>(i)[j]) { pixelCnt++; }
				}

			if (pixelCnt >= (coreSize.height * coreSize.width) / 2 + thresh)
			{
				circle(cornerImage, Point(j, i), 0, 255);
			}
		}
}

void ShuttleRecognizer::matchFeaturePoints(Mat &imageLeft, Mat &imageRight, Size windowSize, int yRange, int thresh)
{
	cvtColor(dstLeft, mtdCornerLeft, CV_GRAY2BGR);
	cvtColor(dstRight, mtdCornerRight, CV_GRAY2BGR);

	//Left image
	for (auto i = windowSize.height / 2 + yRange / 2; i < imageLeft.rows - windowSize.height / 2 - yRange / 2; i++)
		for (auto j = windowSize.width / 2; j < imageLeft.cols - windowSize.width / 2; j++)
			if (imageLeft.ptr<uchar>(i)[j] == 255)
			{
				//Right image
				for (auto m = i - yRange / 2; m <= i + yRange / 2; m++)
					for (auto n = windowSize.width / 2; n < imageLeft.cols - windowSize.width / 2; n++)
						if (imageRight.ptr<uchar>(m)[n] == 255)
						{
							int pixelCnt = 0;

							//Traversal window
							for (auto p = -windowSize.height / 2; p <= windowSize.height / 2; p++)
								for (auto q = -windowSize.width / 2; q <= windowSize.width / 2; q++)
									if (dstLeft.ptr<uchar>(i + p)[j + q] == dstRight.ptr<uchar>(m + p)[n + q])
									{ 
										pixelCnt++; 
									}

							if (pixelCnt >= thresh)
							{
								circle(mtdCornerLeft, Point(j, i), 3, Scalar(0, 0, 255), 2);
								circle(mtdCornerRight, Point(n, m), 3, Scalar(0, 0, 255), 2);
							}
						}
			}

	imshow("mtdL", mtdCornerLeft);
	imshow("mtdR", mtdCornerRight);
}

void ShuttleRecognizer::getConnectedComponent(Mat &binary, Point initialPoint, ConnectedComponent &cc)
{
	std::vector<Point> stkPoint;
	unsigned int counter = 0;
	unsigned long long coreX = 0;
	unsigned long long coreY = 0;
	outerRect ccOuterRect(640, 0, 480, 0);

	stkPoint.push_back(initialPoint);

	//if find a white point connected, which means there are element in stkPoint
	while (!stkPoint.empty())
	{
		auto pix = stkPoint.back();

		stkPoint.pop_back();
		counter++;
		coreX += pix.x;
		coreY += pix.y;
		ccOuterRect.xMin = pix.x < ccOuterRect.xMin ? pix.x : ccOuterRect.xMin;
		ccOuterRect.xMax = pix.x > ccOuterRect.xMax ? pix.x : ccOuterRect.xMax;
		ccOuterRect.yMin = pix.y < ccOuterRect.yMin ? pix.y : ccOuterRect.yMin;
		ccOuterRect.yMax = pix.y > ccOuterRect.yMax ? pix.y : ccOuterRect.yMax;

		auto row_0 = pix.y - 1, row_1 = pix.y, row_2 = pix.y + 1;
		auto col_0 = pix.x - 1, col_1 = pix.x, col_2 = pix.x + 1;

#define PASS(x, y)  do { binary.ptr(y)[x] = 150; stkPoint.push_back({x, y}); } while (0)

		//row_0
		if (row_0 >= 0 && col_0 >= 0 && binary.ptr(row_0)[col_0] == 255)
			PASS(col_0, row_0);
		if (row_0 >= 0 && binary.ptr(row_0)[col_1] == 255)
			PASS(col_1, row_0);
		if (row_0 >= 0 && col_2 < binary.cols && binary.ptr(row_0)[col_2] == 255)
			PASS(col_2, row_0);

		//row_1
		if (col_0 >= 0 && binary.ptr(row_1)[col_0] == 255)
			PASS(col_0, row_1);
		if (col_2 < binary.cols && binary.ptr(row_1)[col_2] == 255)
			PASS(col_2, row_1);

		//row_2
		if (row_2 < binary.rows && col_0 >= 0 && binary.ptr(row_2)[col_0] == 255)
			PASS(col_0, row_2);
		if (row_2 < binary.rows && binary.ptr(row_2)[col_1] == 255)
			PASS(col_1, row_2);
		if (row_2 < binary.rows && col_2 < binary.cols && binary.ptr(row_2)[col_2] == 255)
			PASS(col_2, row_2);

#undef PASS

	}

	coreX /= counter;
	coreY /= counter;
	cc.core = Point((int)coreX, (int)coreY);

	cc.size = counter;
	cc.outerRect = ccOuterRect;
}


STATUS ShuttleRecognizer::findMatchedPoints(Mat &binLeft, ConnectedComponent &scLeft, Mat &binRight, ConnectedComponent &scRight)
{
	auto binaryLeft = binLeft.clone();
	auto binaryRight = binRight.clone();
	Point crtPoint = Point(-1, -1);
	vector<ConnectedComponent> ccsLeft;
	ConnectedComponent tmpCC;
	unsigned int ccCounter = 0;

	for (auto i = 0; i < binaryLeft.rows; ++i)
	{
		for (auto j = 0; j < binaryLeft.cols; ++j)
		{
			if (binaryLeft.ptr(i)[j] == 255)
			{
				binaryLeft.ptr(i)[j] = 150;
				crtPoint = Point(j, i);
			}

			if (crtPoint != Point(-1, -1))
			{
				getConnectedComponent(binaryLeft, crtPoint, tmpCC);
				if ((tmpCC.outerRect.xMax - tmpCC.outerRect.xMin) < 2 * (tmpCC.outerRect.yMax - tmpCC.outerRect.yMin) &&
					(tmpCC.outerRect.yMax - tmpCC.outerRect.yMin) < 3 * (tmpCC.outerRect.xMax - tmpCC.outerRect.xMin) &&
					tmpCC.outerRect.yMax - tmpCC.outerRect.yMin < 200 && tmpCC.outerRect.xMax - tmpCC.outerRect.xMin < 120 && 
					tmpCC.size >= 30)
				{
					ccsLeft.push_back(tmpCC); 
				}
				crtPoint = Point(-1, -1);
			}
		}
	}

	while (!ccsLeft.empty())
	{
		auto refCore = ccsLeft.back().core;
		for (int i = 0; i < 640; i++)
		{
			if (binaryRight.ptr(refCore.y)[i] == 255)
			{
				binaryLeft.ptr(refCore.y)[i] = 150;
				crtPoint = Point(i, refCore.y);
			}

			if (crtPoint != Point(-1, -1))
			{
				getConnectedComponent(binaryRight, crtPoint, tmpCC);

				if (tmpCC.size / ccsLeft.back().size < 3 && ccsLeft.back().size / tmpCC.size < 3)
				{
					locationVec = simulatedLocating(refCore.y, refCore.x, tmpCC.core.y, tmpCC.core.x);
					if (locationVec[0] < 500 && locationVec[0] > -500 && locationVec[1] < 4000 && locationVec[2] > 0 && locationVec[2] < 10000)
					{
						scLeft = ccsLeft.back();
						scRight = tmpCC;

						//cout << "L  " << ccsLeft.back().size << "  " << ccsLeft.back().core << endl;
						//cout << "R  " << tmpCC.size << "  " << tmpCC.core << endl;

						return true;
					}
				}

				crtPoint = Point(-1, -1);
			}
		}

		ccsLeft.pop_back();
	}

	return false;
}
