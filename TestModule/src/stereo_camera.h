#ifndef _STEREO_CAMERA_H
#define _STEREO_CAMERA_H

#define ROTATION_MATRIX		 (Mat_<double>(3, 3) << 1.0000f, -0.0063f, 0.0068f, \
													0.0063f, 1.0000f, 0.0004f, \
													-0.0068f, -0.0004f, 1.0000f)
#define TRANSLATION_MATRIX	 (Mat_<double>(3, 1) << -128.5457f, -0.9355f, 0.7150f)

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

	//用OpenCV内置函数输出测试深度图
	void outputDepthImage();

	//模拟测距函数
	Vec4d simulatedLocating(int yLeft, int xLeft, int yRight, int xRight);

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

	Mat RLeft;				//Rotation Matrix of Left Camera
	Mat RRight;				//Rotation Matrix of Right Camera
	Mat PLeft;				//Projection Matrix of Left Camera
	Mat PRight;				//Projection Matrix of Right Camera
	Mat Q;					//Disparity-to-Depth Mapping Matrix
	Rect validROILeft;
	Rect validROIRight;

	Mat mapLx;
	Mat mapLy;
	Mat mapRx;
	Mat mapRy;
};

#endif //_STEREO_CAMERA_H