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

	//dilate(fgImageLeft, fgImageLeft, element, Point(-1, -1), 3);
	//erode(fgImageLeft, fgImageLeft, element, Point(-1, -1), 6);
	//dilate(fgImageLeft, fgImageLeft, element, Point(-1, -1), 3);

	//dilate(fgImageRight, fgImageRight, element, Point(-1, -1), 3);
	//erode(fgImageRight, fgImageRight, element, Point(-1, -1), 6);
	//dilate(fgImageRight, fgImageRight, element, Point(-1, -1), 3);

	//medianBlur(fgImageLeft, fgImageLeft, 3);
	//medianBlur(fgImageRight, fgImageRight, 3);

	//threshold(fgImageLeft, dstLeft, 0, 255, THRESH_BINARY);
	//threshold(fgImageRight, dstRight, 0, 255, THRESH_BINARY);
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

bool ShuttlecockRecognizer::shuttlecockDetection()
{
	Mat img1 = fgImageLeft;
	Mat img2 = fgImageRight;

	//-- Step 1: Detect the keypoints using SURF Detector, compute the descriptors
	int minHessian = 400;
	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> keypoints1, keypoints2;
	Mat descriptors1, descriptors2;

	detector->detectAndCompute(img1, noArray(), keypoints1, descriptors1);
	detector->detectAndCompute(img2, noArray(), keypoints2, descriptors2);

	//-- Need to check descriptors
	if (descriptors1.rows <= 1 || descriptors2.rows <= 1)
	{
		cout << "Descriptors Empty" << endl;
		imshow("testL", img1);
		imshow("testR", img2);
		return false;
	}

	//-- Step 2: Matching descriptor vectors with a FLANN based matcher
	// Since SURF is a floating-point descriptor NORM_L2 is used
	Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create(DescriptorMatcher::FLANNBASED);
	std::vector<std::vector<DMatch>> knn_matches;
	matcher->knnMatch(descriptors1, descriptors2, knn_matches, 2);

	//-- Filter matches using the Lowe's ratio test
	const float ratio_thresh = 0.7f;
	std::vector<DMatch> good_matches;
	for (size_t i = 0; i < knn_matches.size(); i++)
	{
		if (knn_matches[i][0].distance < ratio_thresh * knn_matches[i][1].distance)
		{
			good_matches.push_back(knn_matches[i][0]);
		}
	}

	//-- Draw matches
	Mat img_matches;
	drawMatches(img1, keypoints1, img2, keypoints2, good_matches, img_matches, Scalar::all(-1),
		Scalar::all(-1), std::vector<char>(), DrawMatchesFlags::NOT_DRAW_SINGLE_POINTS);

	//-- Show detected matches
	imshow("Good Matches", img_matches);

	return true;
}