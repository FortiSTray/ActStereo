#ifndef _STEREO_CAMERA_H
#define _STEREO_CAMERA_H

#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 1.0000f, -0.0063f, 0.0068f, \
													0.0063f, 1.0000f, 0.0004f, \
													-0.0068f, -0.0004f, 1.0000f)
#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << -128.5457f, -0.9355f, 0.7150f)

//#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 1.0f, 0.0f, 0.0f, \
//													0.0f, 1.0f, 0.0f, \
//													0.0f, 0.0f, 1.0f)
//#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << 0.0f, 0.0f, 0.0f)

#include <iostream>
#include <opencv2/opencv.hpp>
#include "camera_module.h"

using namespace std;
using namespace cv;

class StereoCamera
{
public:
	StereoCamera() {}
	explicit StereoCamera(char idLeft, CameraArguments argsLeft, char idRight, CameraArguments argsRight);
	StereoCamera(const StereoCamera&) = delete;
	StereoCamera& operator=(const StereoCamera&) = delete;
	virtual ~StereoCamera() {}

	void updateFrame();

	//Ä£Äâ²â¾àº¯Êý
	double analogRanging(int yLeft, int xLeft, int yRight, int xRight);

	Mat getFrameLeft() { return frameLeft; }
	Mat getFrameRight() { return frameRight; }

private:
	CameraModule cameraLeft;
	CameraModule cameraRight;

	CameraArguments cameraArgsLeft;
	CameraArguments cameraArgsRight;

	Mat frameLeft;
	Mat frameRight;

	Mat rotationMatrix;
	Mat translationMatrix;

	Mat rotationMatrixLeft;
	Mat rotationMatrixRight;
	Mat newMatrixLeft;
	Mat newMatrixRight;
	Mat reprojectionMatrix;
	Rect validROIL;
	Rect validROIR;

	Mat mapLx;
	Mat mapLy;
	Mat mapRx;
	Mat mapRy;
};

#endif //_STEREO_CAMERA_H