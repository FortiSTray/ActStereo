#include "shuttlecock_recognizer.h"

/**
* @brief  该类的一种重载构造函数
* @param  camNameL 左相机名称
* @param  argsLeft 左相机内参
* @param  camNameL 右相机名称
* @param  argsLeft 右相机内参
* @return None
* @note   继承自MVStereo类，调用MVStereo类的构造函数，并为ShuttlecockRecognizer类中用到的的一些变量赋值
* @author ST42_Action (Liang Chen)
*/
ShuttlecockRecognizer::ShuttlecockRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight) :
	MVStereo(camNameL, argsLeft, camNameR, argsRight)
{
	bgModel = createBackgroundSubtractorMOG2(500, 16.0, false).dynamicCast<BackgroundSubtractor>();

	element = getStructuringElement(MORPH_RECT, Size(3, 3));
}

/**
* @brief  背景减除
* @param  None
* @return None
* @note   进行背景减除操作，生成前景图像
* @author ST42_Action (Liang Chen)
*/
void ShuttlecockRecognizer::backgroundSubtract()
{
	bgModel->apply(getFrameLeft(), fgMaskLeft, 0.01);
	bgModel->apply(getFrameRight(), fgMaskRight, 0.01);

	fgImageLeft = Scalar::all(0);
	fgImageRight = Scalar::all(0);

	getFrameLeft().copyTo(fgImageLeft, fgMaskLeft);
	getFrameRight().copyTo(fgImageRight, fgMaskRight);

	imshow("LeftFg", fgImageLeft);
	imshow("RightFg", fgImageRight);
}

/**
* @brief  图像预处理
* @param  None
* @return None
* @note   对要操作的图像进行初步的处理与滤波
* @author ST42_Action (Liang Chen)
*/
void ShuttlecockRecognizer::preprocess()
{
	cvtColor(fgImageLeft, fgGrayLeft, CV_BGR2GRAY);
	cvtColor(fgImageRight, fgGrayRight, CV_BGR2GRAY);

	medianBlur(fgGrayLeft, fgGrayLeft, 3);
	medianBlur(fgGrayRight, fgGrayRight, 3);
}

/**
* @brief  绣球检测
* @param  windowSize 用于左右特征点匹配的窗口大小
* @param  yRange 左右图像进行极线对齐时y坐标允许的误差
* @param  thresh 匹配窗口匹配像素点数量的阈值
* @return STATUS类型，用于判断是否成功检测到目标窗口
* @note   使用SURF特征检测左右图像的特征点，并利用双目相机左右图像极线对齐的特点进行图像匹配，进一步对绣球进行
* @author ST42_Action (Liang Chen)
*/
STATUS ShuttlecockRecognizer::shuttlecockDetection(Size windowSize, int yRange, int thresh)
{
	Mat imageLeft = fgGrayLeft;
	Mat imageRight = fgGrayRight;

	vector<MatchedPoint> mtdPoints;
	MatchedPoint tmpMtdPoint;

	//-- Detect the keypoints using SURF Detector
	int minHessian = 400;
	Ptr<SURF> detector = SURF::create(minHessian);
	std::vector<KeyPoint> keypointsL, keypointsR;

	detector->detect(imageLeft, keypointsL);
	detector->detect(imageRight, keypointsR);

	//-- Draw keypoints
	Mat kpImageLeft, kpImageRight;
	drawKeypoints(imageLeft, keypointsL, kpImageLeft);
	drawKeypoints(imageRight, keypointsR, kpImageRight);

	//-- Show detected keypoints
	imshow("SURF Keypoints Left", kpImageLeft);
	imshow("SURF Keypoints Right", kpImageRight);

	fgImageLeft.copyTo(mtdCornerLeft);
	fgImageRight.copyTo(mtdCornerRight);

	for (size_t i = 0; i < keypointsL.size(); i++)
		for (size_t j = 0; j < keypointsR.size(); j++)
		{
			Point2i point1 = static_cast<Point2i>(keypointsL[i].pt);
			Point2i point2 = static_cast<Point2i>(keypointsR[j].pt);

			if (fabs(point1.y - point2.y) <= yRange &&
				point1.y >= windowSize.height / 2 && point1.y < imageLeft.rows  - windowSize.height / 2 &&
				point1.x >= windowSize.width  / 2 && point1.x < imageLeft.cols  - windowSize.width  / 2 &&
				point2.y >= windowSize.height / 2 && point2.y < imageRight.rows - windowSize.height / 2 &&
				point2.x >= windowSize.width  / 2 && point2.x < imageRight.cols - windowSize.width  / 2)
			{
				int pixelCnt = 0;

				//-- Traversal window
				for (auto p = -windowSize.height / 2; p <= windowSize.height / 2; p++)
					for (auto q = -windowSize.width / 2; q <= windowSize.width / 2; q++)
						if (imageLeft.ptr<uchar>(point1.y + p)[point1.x + q] && imageRight.ptr<uchar>(point2.y + p)[point2.x + q])
						{
							pixelCnt++;
						}

				if (pixelCnt >= thresh)
				{
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

	for (size_t i = 0; i < mtdPoints.size(); i++)
	{
		circle(mtdCornerLeft, mtdPoints[i].pointLeft, 3, Scalar(0, 0, 255), 2);
		circle(mtdCornerRight, mtdPoints[i].pointRight, 3, Scalar(0, 0, 255), 2);

		objectFound = false;

		for (size_t j = 0; j < objRects.size(); j++)
		{
			if (abs(mtdPoints[i].disparity - objRects[j].disparity) < 35)
			{
				objRects[j].yMax = MAX(objRects[j].yMax, mtdPoints[i].pointLeft.y);
				objRects[j].xMax = MAX(objRects[j].xMax, mtdPoints[i].pointLeft.x);
				objRects[j].yMin = MIN(objRects[j].yMin, mtdPoints[i].pointLeft.y);
				objRects[j].xMin = MIN(objRects[j].xMin, mtdPoints[i].pointLeft.x);

				objRects[j].pointNum++;
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

	for (size_t i = 0; i < objRects.size(); i++)
	{
		if (objRects[i].pointNum <= 9) { continue; }

		line(mtdCornerLeft, Point(objRects[i].xMin, objRects[i].yMin), Point(objRects[i].xMax, objRects[i].yMin), Scalar(0, 255, 0), 2);
		line(mtdCornerLeft, Point(objRects[i].xMax, objRects[i].yMin), Point(objRects[i].xMax, objRects[i].yMax), Scalar(0, 255, 0), 2);
		line(mtdCornerLeft, Point(objRects[i].xMax, objRects[i].yMax), Point(objRects[i].xMin, objRects[i].yMax), Scalar(0, 255, 0), 2);
		line(mtdCornerLeft, Point(objRects[i].xMin, objRects[i].yMax), Point(objRects[i].xMin, objRects[i].yMin), Scalar(0, 255, 0), 2);
	}
	
	imshow("mtdL", mtdCornerLeft);
	imshow("mtdR", mtdCornerRight);

	if (!objRects.empty())
	{
		detectWindow.x = objRects[0].xMin;
		detectWindow.y = objRects[0].yMin;
		detectWindow.width = abs(objRects[0].xMax - objRects[0].xMin);
		detectWindow.height = abs(objRects[0].yMax - objRects[0].yMin);
		detectWindow &= Rect(0, 0, SRC_COLS, SRC_ROWS);

		isObjectTracked = -1;

		return true;
	}
	else { return false; }
}

/**
* @brief  绣球跟踪
* @param  None
* @return STATUS类型，用于判断目标成功跟踪或者丢失
* @note   在检测到绣球窗口的基础上，使用CAMShift算法对绣球进行跟踪
* @author ST42_Action (Liang Chen)
*/
STATUS ShuttlecockRecognizer::shuttlecockTracking()
{
	if (isObjectTracked)
	{
		cvtColor(fgImageLeft, hsvImage, COLOR_BGR2HSV);

		inRange(hsvImage, Scalar(0, 30, 40), Scalar(180, 255, 255), svMask);
		int ch[] = { 0, 0 };
		hueImage.create(hsvImage.size(), hsvImage.depth());
		mixChannels(&hsvImage, 1, &hueImage, 1, ch, 1);

		imshow("hsvImage", hueImage);

		if (isObjectTracked < 0)
		{
			// Object has been selected by user, set up CAMShift search properties once
			Mat roi(hueImage, detectWindow), maskroi(svMask, detectWindow);
			calcHist(&roi, 1, 0, maskroi, hist, 1, &hSize, &phranges);
			normalize(hist, hist, 0, 255, NORM_MINMAX);

			trackWindow = detectWindow;
			isObjectTracked = 1;

			//-- Draw histogram
			histImage = drawHist(hist, hSize);
			imshow("Histogram", histImage);
		}

		//-- Perform CAMShift
		if (!trackWindow.area()) { return false; }

		calcBackProject(&hueImage, 1, 0, hist, backproj, &phranges);
		backproj &= svMask;
		RotatedRect trackBox = CamShift(backproj, trackWindow,
			TermCriteria(TermCriteria::EPS | TermCriteria::COUNT, 10, 1));
		if (trackWindow.area() <= 1)
		{
			int cols = backproj.cols, rows = backproj.rows, r = (MIN(cols, rows) + 5) / 6;
			trackWindow = Rect(trackWindow.x - r, trackWindow.y - r,
				trackWindow.x + r, trackWindow.y + r) &
				Rect(0, 0, cols, rows);
		}
		if (!trackBox.size.area()) { return false; }

		ellipse(fgImageLeft, trackBox, Scalar(0, 0, 255), 3, LINE_AA);
	}

	imshow("mtdL", fgImageLeft);
	imshow("mtdR", fgImageRight);

	if (trackWindow.area() > 50) { return true; }
	else { return false; }
}

/**
* @brief  绘制直方图
* @param  hist 需要绘制的直方图
* @param  hsize 直方图bin的数量
* @return 返回该函数绘制出的直方图
* @author ST42_Action (Liang Chen)
*/
Mat ShuttlecockRecognizer::drawHist(Mat hist, int hsize)
{
	Mat histImg = Mat::zeros(200, 320, CV_8UC3);
	int binWidth = histImg.cols / hsize;
	Mat buffer(1, hsize, CV_8UC3);

	for (int i = 0; i < hsize; i++)
	{
		buffer.at<Vec3b>(i) = Vec3b(saturate_cast<uchar>(i * 180.0f / hsize), 255, 255);
	}
	cvtColor(buffer, buffer, COLOR_HSV2BGR);

	for (int i = 0; i < hsize; i++)
	{
		int val = saturate_cast<int>(hist.at<float>(i) * histImg.rows / 255);
		rectangle(histImg, Point(i * binWidth, histImg.rows), Point((i + 1) * binWidth, histImg.rows - val),
			Scalar(buffer.at<Vec3b>(i)), -1, 8);
	}

	return histImg;
}