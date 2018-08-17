#ifndef _SHUTTLECOCK_RECOGNIZER_H
#define _SHUTTLECOCK_RECOGNIZER_H

#include "mv_stereo.h"
#include "opencv2/xfeatures2d.hpp"

using namespace cv;
using namespace cv::xfeatures2d;

typedef bool STATUS;

//-- Matched Points
typedef struct
{
	Point2i pointLeft;
	Point2i pointRight;
	int		disparity;

} MatchedPoint;

//-- Object Rect
struct ObjectRect
{
	ObjectRect() {}
	ObjectRect(int l, int r, int u, int d) : xMin(l), xMax(r), yMin(u), yMax(d) {}

	int xMin = SRC_COLS;
	int xMax = 0;
	int yMin = SRC_ROWS;
	int yMax = 0;

	int disparity;
	int pointNum;
};

class ShuttlecockRecognizer : public MVStereo
{
public:
	ShuttlecockRecognizer() {}
	explicit ShuttlecockRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight);
	ShuttlecockRecognizer(const ShuttlecockRecognizer&) = delete;
	ShuttlecockRecognizer& operator=(const ShuttlecockRecognizer&) = delete;
	virtual ~ShuttlecockRecognizer() {}

	//-- ±³¾°¼õ³ý
	void backgroundSubtract();

	//-- Í¼ÏñÔ¤´¦Àí
	void preprocess();

	//-- ÐåÇò¼ì²â
	STATUS shuttlecockDetection(Size windowSize, int yRange, int thresh);

	//-- ÐåÇò¸ú×Ù
	STATUS shuttlecockTracking();

	//-- Ö±·½Í¼»æÖÆ
	Mat drawHist(Mat hist, int hsize);

	Mat getFgImageLeft() { return fgImageLeft; }
	Mat getFgImageRight() { return fgImageRight; }

	Mat getFgGrayLeft() { return fgGrayLeft; }
	Mat getFgGrayRight() { return fgGrayRight; }

	Mat getTestImage() { return testImage; }

private:
	Mat fgMaskLeft;
	Mat fgMaskRight;
	Mat fgImageLeft;
	Mat fgImageRight;
	Mat fgGrayLeft;
	Mat fgGrayRight;
	Mat mtdCornerLeft;
	Mat mtdCornerRight;
	Mat testImage;

	//-- Background Subtract
	Ptr<BackgroundSubtractor> bgModel;
	
	//-- Pre-Processing
	Mat element;

	//-- Tracking
	Rect detectWindow;
	Rect trackWindow;
	int isObjectTracked = 0;
	int hSize = 16;
	float hranges[2] = { 0, 180 };
	const float* phranges = hranges;

	Mat hsvImage;
	Mat hueImage;
	Mat svMask;
	Mat hist;
	Mat histImage = Mat::zeros(200, 320, CV_8UC3);
	Mat backproj;
};

#endif //_SHUTTLECOCK_RECOGNIZER_H