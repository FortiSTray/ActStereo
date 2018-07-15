#include <iostream>
#include <opencv2/opencv.hpp>
#include "camera_module.h"
#include "timer.h"

using namespace std;
using namespace cv;

int main(int argc, char* argv[])
{
	CameraArguments cameraArgsLeft = CAMERA_ARGS_LEFT;
	CameraArguments cameraArgsRight = CAMERA_ARGS_RIGHT;

	CameraModule cameraLeft(2, cameraArgsLeft);
	CameraModule cameraRight(1, cameraArgsRight);
	
	//test block
	Mat rotationMatrix = (Mat_<double>(3, 3) << 0.9999f, -0.0054f, -0.0099f, 0.0054f, 1.000f, 0.0019f, 0.0098f, -0.0019f, 0.9999f);
	Mat translationMatrix = (Mat_<double>(3, 1) << 128.5681f, 1.0181f, 1.8211f);

	Mat Rl, Rr, Pl, Pr, Q;
	stereoRectify(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, 
		Size(640, 480), rotationMatrix, translationMatrix, Rl, Rr, Pl, Pr, Q);

	Mat mapLx, mapLy, mapRx, mapRy;
	initUndistortRectifyMap(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, Rl, Pl, Size(640, 480), CV_32FC1, mapLx, mapLy);
	initUndistortRectifyMap(cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, Rr, Pr, Size(640, 480), CV_32FC1, mapRx, mapRy);

	/*initUndistortRectifyMap(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, Mat(),
		getOptimalNewCameraMatrix(cameraLeft.intrinsicMatrix, cameraLeft.distortionCoeff, Size(640, 480), 1, Size(640, 480), 0),
		Size(640, 480), CV_32FC1, mapLx, mapLy);

	initUndistortRectifyMap(cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, Mat(),
		getOptimalNewCameraMatrix(cameraRight.intrinsicMatrix, cameraRight.distortionCoeff, Size(640, 480), 1, Size(640, 480), 0),
		Size(640, 480), CV_32FC1, mapRx, mapRy);*/

	Mat testL, testR;

	Mat left3 = imread("Left3.png");
	Mat right3 = imread("Right3.png");
	remap(left3, testL, mapLx, mapLy, INTER_LINEAR);
	remap(right3, testR, mapRx, mapRy, INTER_LINEAR);
	line(testL, cvPoint(0, 48), cvPoint(640 - 1, 48), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 2), cvPoint(640 - 1, 48 * 2), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 3), cvPoint(640 - 1, 48 * 3), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 4), cvPoint(640 - 1, 48 * 4), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 5), cvPoint(640 - 1, 48 * 5), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 6), cvPoint(640 - 1, 48 * 6), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 7), cvPoint(640 - 1, 48 * 7), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 8), cvPoint(640 - 1, 48 * 8), cvScalar(255, 0, 0));
	line(testL, cvPoint(0, 48 * 9), cvPoint(640 - 1, 48 * 9), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48), cvPoint(640 - 1, 48), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 2), cvPoint(640 - 1, 48 * 2), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 3), cvPoint(640 - 1, 48 * 3), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 4), cvPoint(640 - 1, 48 * 4), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 5), cvPoint(640 - 1, 48 * 5), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 6), cvPoint(640 - 1, 48 * 6), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 7), cvPoint(640 - 1, 48 * 7), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 8), cvPoint(640 - 1, 48 * 8), cvScalar(255, 0, 0));
	line(testR, cvPoint(0, 48 * 9), cvPoint(640 - 1, 48 * 9), cvScalar(255, 0, 0));
	imshow("Camera Left", testL);
	imshow("Camera Right", testR);
	waitKey();

	while (true)
	{
		cameraLeft.updateFrame();
		cameraRight.updateFrame();

		/*imshow("Camera Left", cameraLeft.getSrcImage());
		imshow("Camera Right", cameraRight.getSrcImage());*/

		remap(cameraLeft.getSrcImage(), testL, mapLx, mapLy, INTER_LINEAR);
		remap(cameraRight.getSrcImage(), testR, mapRx, mapRy, INTER_LINEAR);

		line(testL, cvPoint(0, 48), cvPoint(640 - 1, 48), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 2), cvPoint(640 - 1, 48 * 2), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 3), cvPoint(640 - 1, 48 * 3), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 4), cvPoint(640 - 1, 48 * 4), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 5), cvPoint(640 - 1, 48 * 5), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 6), cvPoint(640 - 1, 48 * 6), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 7), cvPoint(640 - 1, 48 * 7), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 8), cvPoint(640 - 1, 48 * 8), cvScalar(255, 0, 0));
		line(testL, cvPoint(0, 48 * 9), cvPoint(640 - 1, 48 * 9), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48), cvPoint(640 - 1, 48), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 2), cvPoint(640 - 1, 48 * 2), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 3), cvPoint(640 - 1, 48 * 3), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 4), cvPoint(640 - 1, 48 * 4), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 5), cvPoint(640 - 1, 48 * 5), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 6), cvPoint(640 - 1, 48 * 6), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 7), cvPoint(640 - 1, 48 * 7), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 8), cvPoint(640 - 1, 48 * 8), cvScalar(255, 0, 0));
		line(testR, cvPoint(0, 48 * 9), cvPoint(640 - 1, 48 * 9), cvScalar(255, 0, 0));

		imshow("Camera Left", testL);
		imshow("Camera Right", testR);

		//if push down Esc, kill the progress
		if (waitKey(1) == 0x1B)
		{
			break;
		}
	}

	return 0;
}