#ifndef _CAMERA_MODULE_H
#define _CAMERA_MODULE_H

//Arguments of left camera
#define CAMERA_ARGS_LEFT  { 529.6793f, 530.9351f,    /*Focal Length*/ \
							325.8743f, 215.4840f,    /*Principal Point*/ \
							-1.3523f,    /*Skew*/ \
							-0.4310f, 0.2169f,   /*Radial Distortion*/ \
							-0.0016f, 8.1329e-05f   /*Tangential Distortion*/ }

//Arguments of right camera
#define CAMERA_ARGS_RIGHT { 535.0516f, 536.7354f,    /*Focal Length*/ \
							293.0789f, 239.2839f,    /*Principal Point*/ \
							-1.1520f,    /*Skew*/ \
							-0.4341f, 0.2194f,   /*Radial Distortion*/ \
							-0.0013f, -6.8871e-04f   /*Tangential Distortion*/ }

//Source Image Size
#define SRC_ROWS			480
#define SRC_COLS			640

//Exposure
#define EXPOSURE_VALUE		-6

#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//Struct of camera arguments
typedef struct
{
	//inner arguments
	float fx;
	float fy;
	float cx;
	float cy;
	float skew;

	//distortion arguments
	float k1;
	float k2;
	float p1;
	float p2;

} CameraArguments;

class CameraModule
{
public:

	CameraModule() {}
	explicit CameraModule(char cameraId, CameraArguments cameraArgs);
	CameraModule(const CameraModule&) = delete;
	CameraModule& operator=(CameraModule&) = delete;
	virtual~CameraModule() {}

	void operator()(char cameraId, CameraArguments cameraArgs);

	bool isOpened() { return !cameraCapture.isOpened(); }

	void updateFrame();

	Mat getFrame() { return frame; }

	int rows;
	int cols;

	Mat intrinsicMatrix;
	Mat distortionCoeff;

private:

	VideoCapture cameraCapture;

	CameraArguments args;

	Mat srcImage;
	Mat frame;
};


#endif//_CAMERA_MODULE_H