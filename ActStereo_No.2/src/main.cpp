#include <iostream>
#include <opencv2/opencv.hpp>
#include "mv_stereo.h"
#include "timer.h"

using namespace std;
using namespace cv;

Timer periodTimer;

//Vec4d testVec;
//
//const int g_xTrackbarMax = 640;
//const int g_yTrackbarMax = 480;
//int g_xLeftSlider = 300;
//int g_yLeftSlider = 226;
//int g_xRightSlider = 320;
//int g_yRightSlider = 226;
//void trackbarCallback(int, void*) {}

int main(int argc, char* argv[])
{
	MVStereo stStereoNo_1("CameraLeft", CAMERA_ARGS_LEFT, "CameraRight", CAMERA_ARGS_RIGHT);

	stStereoNo_1.stereoInit();

	//cv::namedWindow("TKB");
	//createTrackbar("xLeft", "TKB", &g_xLeftSlider, g_xTrackbarMax, trackbarCallback);
	//createTrackbar("yLeft", "TKB", &g_yLeftSlider, g_yTrackbarMax, trackbarCallback);
	//createTrackbar("xRight", "TKB", &g_xRightSlider, g_xTrackbarMax, trackbarCallback);
	//createTrackbar("yRight", "TKB", &g_yRightSlider, g_yTrackbarMax, trackbarCallback);

	while (true)
	{
		stStereoNo_1.syncUpdate();

		//testVec = stStereoNo_1.simulatedLocating(g_yLeftSlider, g_xLeftSlider, g_yRightSlider, g_xRightSlider);
		//cout << "x = " << testVec[0] << "\ty = " << testVec[1] << "\tz = " << testVec[2] << "\td = " << testVec[3] << endl;

		cout << periodTimer.end() << endl;
		periodTimer.begin();

		//if push down Esc, kill the progress
		if (waitKey(1) == 0x1B)
		{
			break;
		}
	}

	return 0;
}