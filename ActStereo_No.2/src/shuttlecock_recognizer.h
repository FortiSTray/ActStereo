#ifndef _SHUTTLECOCK_RECOGNIZER_H
#define _SHUTTLECOCK_RECOGNIZER_H

#include "mv_stereo.h"

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

class ShuttleRecognizer : public MVStereo
{
public:
	ShuttleRecognizer() {}
	explicit ShuttleRecognizer(char* camNameL, CameraArguments argsLeft, char* camNameR, CameraArguments argsRight);
	ShuttleRecognizer(const ShuttleRecognizer&) = delete;
	ShuttleRecognizer& operator=(const ShuttleRecognizer&) = delete;
	virtual ~ShuttleRecognizer() {}

	//背景减除
	void backgroundSubtract();

	void preProc();

	void detectCorner(Mat &image, Mat &cornerImage, Size coreSize, int thresh = 1);
	
	void matchFeaturePoints(Mat &imageLeft, Mat &imageRight, Size windowSize, int yRange = 3, int thresh = 10);

	//以一个点为种子点获取连通域
	void getConnectedComponent(Mat &binary, Point initialPoint, ConnectedComponent &cc);

	STATUS findMatchedPoints(Mat &binLeft, ConnectedComponent &scLeft, Mat &binRight, ConnectedComponent &scRight);

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