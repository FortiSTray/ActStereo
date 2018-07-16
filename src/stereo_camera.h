#ifndef _STEREO_CAMERA_H
#define _STEREO_CAMERA_H

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