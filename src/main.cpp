#include <iostream>
#include <opencv2/opencv.hpp>
#include "camera_module.h"
#include "stereo_camera.h"
#include "timer.h"

using namespace std;
using namespace cv;

Vec4d testVec;

const int g_xTrackbarMax = 640;
const int g_yTrackbarMax = 480;
int g_xLeftSlider;
int g_yLeftSlider;
int g_xRightSlider;
int g_yRightSlider;
void trackbarCallback(int, void*) {}

int main(int argc, char* argv[])
{
	StereoCamera stStereoNo_1(0, CAMERA_ARGS_LEFT, 1, CAMERA_ARGS_RIGHT);

	cv::namedWindow("TKB");
	createTrackbar("xLeft", "TKB", &g_xLeftSlider, g_xTrackbarMax, trackbarCallback);
	createTrackbar("yLeft", "TKB", &g_yLeftSlider, g_yTrackbarMax, trackbarCallback);
	createTrackbar("xRight", "TKB", &g_xRightSlider, g_xTrackbarMax, trackbarCallback);
	createTrackbar("yRight", "TKB", &g_yRightSlider, g_yTrackbarMax, trackbarCallback);

	while (true)
	{
		stStereoNo_1.updateFrame();

		testVec = stStereoNo_1.simulatedLocating(g_yLeftSlider, g_xLeftSlider, g_yRightSlider, g_xRightSlider);
		cout << "x = " << testVec[0] << "\ty = " << testVec[1] << "\tz = " << testVec[2] << "\td = " << testVec[3] << endl;

		imshow("Left", stStereoNo_1.getFrameLeft());
		imshow("Right", stStereoNo_1.getFrameRight());

		//if push down Esc, kill the progress
		if (waitKey(1) == 0x1B)
		{
			break;
		}
	}

	return 0;
}