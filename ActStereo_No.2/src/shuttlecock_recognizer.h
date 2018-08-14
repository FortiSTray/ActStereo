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

	//hsv空间绣球检测
	bool shuttlecockDetection();

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