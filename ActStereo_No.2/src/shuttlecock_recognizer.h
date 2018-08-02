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

	//��������
	void backgroundSubtract();

	void preProcessing();

	//��һ����Ϊ���ӵ��ȡ��ͨ��
	void getConnectedComponent(Mat &binary, Point initialPoint, ConnectedComponent &cc);

	STATUS findMatchedPoints(Mat &binLeft, ConnectedComponent &scLeft, Mat &binRight, ConnectedComponent &scRight);

	Mat getFgImageLeft() { return fgImageLeft; }
	Mat getFgImageRight() { return fgImageRight; }

	Mat getDstLeft() { return dstLeft; }
	Mat getDstRight() { return dstRight; }

private:
	Mat fgMaskLeft;
	Mat fgMaskRight;
	Mat fgImageLeft;
	Mat fgImageRight;
	Mat dstLeft;
	Mat dstRight;

	Ptr<BackgroundSubtractor> bgModel;

	Mat element;
};

#endif //_SHUTTLECOCK_RECOGNIZER_H