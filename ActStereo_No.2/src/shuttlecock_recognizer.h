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

	//-- ±≥æ∞ºı≥˝
	void backgroundSubtract();

	//-- ÕºœÒ‘§¥¶¿Ì
	void preprocess();

	//-- –Â«ÚºÏ≤‚
	STATUS shuttlecockDetection(Size windowSize, int yRange, int thresh);

	//-- –Â«Ú∏˙◊Ÿ
	STATUS shuttlecockTracking();

	//-- ø®∂˚¬¸¬À≤®œ‡πÿ
	void initKalmanFilter();
	void setCurrentTrackWindow();

	//-- –Â«Ú∆•≈‰
	STATUS shuttlecockMatching();

	//-- ÷±∑ΩÕºªÊ÷∆
	Mat drawHist(Mat hist, int hsize);

	Mat getFgImageLeft() { return fgImageLeft; }
	Mat getFgImageRight() { return fgImageRight; }

	Mat getFgGrayLeft() { return fgGrayLeft; }
	Mat getFgGrayRight() { return fgGrayRight; }

	Mat getTestImage() { return testImage; }

	void reverseBackprojMode() { backprojMode = !backprojMode; }

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
	bool backprojMode = 0;
	int hSize = 16;
	float hranges[2] = { 0, 180 };
	const float* phranges = hranges;

	//-- Kalman Filter
	KalmanFilter KF;
	Mat_<float> measurement;
	Point measureCenter;
	Point predictCenter;
	Point correctCenter;

	//-- Matching
	Mat templateImage;
	Rect searchWidnow;
	Mat searchImage;
	Rect matchedWindow;

	Mat hsvImage;
	Mat hueImage;
	Mat svMask;
	Mat hist;
	Mat histImage = Mat::zeros(200, 320, CV_8UC3);
	Mat backproj;

	int vMin;
	int vMax;
	int sMin;
};

#endif //_SHUTTLECOCK_RECOGNIZER_H