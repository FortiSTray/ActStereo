#ifndef _SHUTTLECOCK_RECOGNIZER_H
#define _SHUTTLECOCK_RECOGNIZER_H

#include "mv_stereo.h"
#include "opencv2/xfeatures2d.hpp"

using namespace cv::xfeatures2d;

typedef bool STATUS;

//Outer Rect
struct outerRect
{
	outerRect() {}
	outerRect(int l, int r, int u, int d) : xMin(l), xMax(r), yMin(u), yMax(d) {}

	int xMin = 0;
	int xMax = 0;
	int yMin = 0;
	int yMax = 0;
};

//Connected Component
typedef struct
{
	int size;
	Point core;
	outerRect outerRect;

} ConnectedComponent;

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

	//背景减除
	void backgroundSubtract();

	void preProcessing();

	//以一个点为种子点获取连通域
	void getConnectedComponent(Mat &binary, Point initialPoint, ConnectedComponent &cc);

	//绣球检测
	bool shuttlecockDetection(Size windowSize, int yRange, int thresh);

	Mat getFgImageLeft() { return fgImageLeft; }
	Mat getFgImageRight() { return fgImageRight; }

	Mat getDstLeft() { return dstLeft; }
	Mat getDstRight() { return dstRight; }

	Mat getCornerLeft() { return cornerLeft; }
	Mat getCornerRight() { return cornerRight; }

	Mat getTestImage() { return testImage; }

private:
	Mat fgMaskLeft;
	Mat fgMaskRight;
	Mat fgImageLeft;
	Mat fgImageRight;
	Mat dstLeft;
	Mat dstRight;
	Mat cornerLeft;
	Mat cornerRight;
	Mat mtdCornerLeft;
	Mat mtdCornerRight;

	Ptr<BackgroundSubtractor> bgModel;

	Mat element;

	Vec4d locationVec;

	Mat testImage;
};

#endif //_SHUTTLECOCK_RECOGNIZER_H