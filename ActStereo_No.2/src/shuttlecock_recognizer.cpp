#include "shuttlecock_recognizer.h"

ShuttlecockRecognizer::ShuttlecockRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight) :
	MVStereo(camNameL, argsLeft, camNameR, argsRight)
{
	bgModel = createBackgroundSubtractorMOG2(500, 16.0, false).dynamicCast<BackgroundSubtractor>();

	element = getStructuringElement(MORPH_RECT, Size(3, 3));

	cornerLeft = Mat::zeros(SRC_ROWS, SRC_COLS, CV_8UC1);
	cornerRight = Mat::zeros(SRC_ROWS, SRC_COLS, CV_8UC1);
}

void ShuttlecockRecognizer::backgroundSubtract()
{
	bgModel->apply(getFrameLeft(), fgMaskLeft, 0.001);
	bgModel->apply(getFrameRight(), fgMaskRight, 0.001);

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

void ShuttlecockRecognizer::preProcessing()
{
	cvtColor(fgImageLeft, fgImageLeft, CV_BGR2GRAY);
	cvtColor(fgImageRight, fgImageRight, CV_BGR2GRAY);

	medianBlur(fgImageLeft, fgImageLeft, 3);
	medianBlur(fgImageRight, fgImageRight, 3);
}

void ShuttlecockRecognizer::getConnectedComponent(Mat &binary, Point initialPoint, ConnectedComponent &cc)
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

bool ShuttlecockRecognizer::shuttlecockDetection(Size windowSize, int yRange, int thresh)
{
	Mat img1 = fgImageLeft;
	Mat img2 = fgImageRight;

	vector<MatchedPoint> mtdPoints;
	MatchedPoint tmpMtdPoint;

	//-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
	int minHessian = 400;
	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> keypoints1, keypoints2;

	detector->detect(img1, keypoints1);
	detector->detect(img2, keypoints2);

	//-- Draw keypoints
	Mat img1_keypoints, img2_keypoints;
	drawKeypoints(img1, keypoints1, img1_keypoints);
	drawKeypoints(img2, keypoints2, img2_keypoints);

	//-- Show detected (drawn) keypoints
	imshow("SURF Keypoints1", img1_keypoints);
	imshow("SURF Keypoints2", img2_keypoints);

	cvtColor(img1, mtdCornerLeft, CV_GRAY2BGR);
	cvtColor(img2, mtdCornerRight, CV_GRAY2BGR);

	for (auto i = 0; i < keypoints1.size(); i++)
		for (auto j = 0; j < keypoints2.size(); j++)
		{
			Point2i point1 = static_cast<Point2i>(keypoints1[i].pt);
			Point2i point2 = static_cast<Point2i>(keypoints2[j].pt);

			if (fabs(point1.y - point2.y) <= yRange &&
				point1.y >= windowSize.height / 2 && point1.y < img1.rows - windowSize.height / 2 &&
				point1.x >= windowSize.width / 2 && point1.x < img1.cols - windowSize.width / 2 &&
				point2.y >= windowSize.height / 2 && point2.y < img2.rows - windowSize.height / 2 &&
				point2.x >= windowSize.width / 2 && point2.x < img2.cols - windowSize.width / 2)
			{
				int pixelCnt = 0;

				//-- Traversal window
				for (auto p = -windowSize.height / 2; p <= windowSize.height / 2; p++)
					for (auto q = -windowSize.width / 2; q <= windowSize.width / 2; q++)
						if (img1.ptr<uchar>(point1.y + p)[point1.x + q] && img2.ptr<uchar>(point2.y + p)[point2.x + q])
						{
							pixelCnt++;
						}

				if (pixelCnt >= thresh)
				{
					//circle(mtdCornerLeft, point1, 3, Scalar(0, 0, 255), 2);
					//circle(mtdCornerRight, point2, 3, Scalar(0, 0, 255), 2);

					tmpMtdPoint.pointLeft = point1;
					tmpMtdPoint.pointRight = point2;
					tmpMtdPoint.disparity = point1.x - point2.x;
					mtdPoints.push_back(tmpMtdPoint);

					break;
				}
			}
		}

	//-- Draw object window
	vector<ObjectRect> objRects;
	bool objectFound = false;

	for (auto i = 0; i < mtdPoints.size(); i++)
	{
		circle(mtdCornerLeft, mtdPoints[i].pointLeft, 3, Scalar(0, 0, 255), 2);
		circle(mtdCornerRight, mtdPoints[i].pointRight, 3, Scalar(0, 0, 255), 2);

		objectFound = false;

		for (auto j = 0; j < objRects.size(); j++)
		{
			if (abs(mtdPoints[i].disparity - objRects[j].disparity) < 35)
			{
#define PASS(max, min) do { if (max < min) { int tmp = max; max = min; min = tmp; } } while(0)

				PASS(objRects[j].yMax, mtdPoints[i].pointLeft.y);
				PASS(objRects[j].xMax, mtdPoints[i].pointLeft.x);
				PASS(mtdPoints[i].pointLeft.y, objRects[j].yMin);
				PASS(mtdPoints[i].pointLeft.x, objRects[j].xMin);
				objRects[j].pointNum++;

#undef PASS
				objectFound = true;
				break;
			}
		}

		if (objectFound == false)
		{
			ObjectRect tmpObjRect(mtdPoints[i].pointLeft.x, mtdPoints[i].pointLeft.x, mtdPoints[i].pointLeft.y, mtdPoints[i].pointLeft.y);
			tmpObjRect.disparity = mtdPoints[i].disparity;
			tmpObjRect.pointNum = 1;
			objRects.push_back(tmpObjRect);
		}
	}

	for (auto i = 0; i < objRects.size(); i++)
	{
		if (objRects[i].pointNum <= 10) { continue; }

		line(mtdCornerLeft, Point(objRects[i].xMin, objRects[i].yMin), Point(objRects[i].xMax, objRects[i].yMin), Scalar(0, 255, 0), 2);
		line(mtdCornerLeft, Point(objRects[i].xMax, objRects[i].yMin), Point(objRects[i].xMax, objRects[i].yMax), Scalar(0, 255, 0), 2);
		line(mtdCornerLeft, Point(objRects[i].xMax, objRects[i].yMax), Point(objRects[i].xMin, objRects[i].yMax), Scalar(0, 255, 0), 2);
		line(mtdCornerLeft, Point(objRects[i].xMin, objRects[i].yMax), Point(objRects[i].xMin, objRects[i].yMin), Scalar(0, 255, 0), 2);
	}


	imshow("mtdL", mtdCornerLeft);
	imshow("mtdR", mtdCornerRight);

	std::vector< DMatch > matches;
	
	return true;
}