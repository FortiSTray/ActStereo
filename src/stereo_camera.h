#ifndef _STEREO_CAMERA_H
#define _STEREO_CAMERA_H

#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 0.9999f, -0.0054f, -0.0099f, \
													0.0054f, 1.000f, 0.0019f, \
													0.0098f, -0.0019f, 0.9999f)
#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << 128.5681f, 1.0181f, 1.8211f)

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

	Mat mapLx;
	Mat mapLy;
	Mat mapRx;
	Mat mapRy;
};

#endif //_STEREO_CAMERA_H