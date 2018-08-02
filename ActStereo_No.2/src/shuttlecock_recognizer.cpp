#include "shuttlecock_recognizer.h"

ShuttleRecognizer::ShuttleRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight) :
	MVStereo(camNameL, argsLeft, camNameR, argsRight)
{
	bgModel = createBackgroundSubtractorMOG2().dynamicCast<BackgroundSubtractor>();

	element = getStructuringElement(MORPH_RECT, Size(10, 10));
}

void ShuttleRecognizer::backgroundSubtract()
{
	bgModel->apply(getFrameLeft(), fgMaskLeft, -1);
	bgModel->apply(getFrameRight(), fgMaskRight, -1);

	//Filter
	//GaussianBlur(fgMaskLeft, fgMaskLeft, Size(11, 11), 3.5, 3.5);
	threshold(fgMaskLeft, fgMaskLeft, 10, 255, THRESH_BINARY);
	//GaussianBlur(fgMaskRight, fgMaskRight, Size(11, 11), 3.5, 3.5);
	threshold(fgMaskRight, fgMaskRight, 10, 255, THRESH_BINARY);

	fgImageLeft = Scalar::all(0);
	getFrameLeft().copyTo(fgImageLeft, fgMaskLeft);
	fgImageRight = Scalar::all(0);
	getFrameRight().copyTo(fgImageRight, fgMaskRight);

	imshow("LeftFg", fgImageLeft);
	imshow("RightFg", fgImageRight);
}

void ShuttleRecognizer::preProcessing()
{
	dilate(fgImageLeft, fgImageLeft, element, Point(-1, -1), 1);
	erode(fgImageLeft, fgImageLeft, element, Point(-1, -1), 1);

	dilate(fgImageRight, fgImageRight, element, Point(-1, -1), 1);
	erode(fgImageRight, fgImageRight, element, Point(-1, -1), 1);

	medianBlur(fgImageLeft, fgImageLeft, 5);
	medianBlur(fgImageLeft, fgImageLeft, 5);

	cvtColor(fgImageLeft, dstLeft, CV_BGR2GRAY);
	cvtColor(fgImageRight, dstRight, CV_BGR2GRAY);

	threshold(dstLeft, dstLeft, 0, 255, THRESH_BINARY);
	threshold(dstRight, dstRight, 0, 255, THRESH_BINARY);
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

#define __pass__(x, y)  do { binary.ptr(y)[x] = 150; stkPoint.push_back({x, y}); } while (0)

		//row_0
		if (row_0 >= 0 && col_0 >= 0 && binary.ptr(row_0)[col_0] == 255)
			__pass__(col_0, row_0);
		if (row_0 >= 0 && binary.ptr(row_0)[col_1] == 255)
			__pass__(col_1, row_0);
		if (row_0 >= 0 && col_2 < binary.cols && binary.ptr(row_0)[col_2] == 255)
			__pass__(col_2, row_0);

		//row_1
		if (col_0 >= 0 && binary.ptr(row_1)[col_0] == 255)
			__pass__(col_0, row_1);
		if (col_2 < binary.cols && binary.ptr(row_1)[col_2] == 255)
			__pass__(col_2, row_1);

		//row_2
		if (row_2 < binary.rows && col_0 >= 0 && binary.ptr(row_2)[col_0] == 255)
			__pass__(col_0, row_2);
		if (row_2 < binary.rows && binary.ptr(row_2)[col_1] == 255)
			__pass__(col_1, row_2);
		if (row_2 < binary.rows && col_2 < binary.cols && binary.ptr(row_2)[col_2] == 255)
			__pass__(col_2, row_2);

#undef __pass__

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
				if (tmpCC.size >= 30) { ccsLeft.push_back(tmpCC); }
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
					scLeft = ccsLeft.back();
					scRight = tmpCC;

					//cout << "L  " << ccsLeft.back().size << "  " << ccsLeft.back().core << endl;
					//cout << "R  " << tmpCC.size << "  " << tmpCC.core << endl;

					return true;
				}

				crtPoint = Point(-1, -1);
			}
		}

		ccsLeft.pop_back();
	}

	return false;
}
