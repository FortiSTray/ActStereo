#include <iostream>
#include <fstream>
#include "opencv2/opencv.hpp"
#include "shuttlecock_recognizer.h"
#include "timer.h"

using namespace std;
using namespace cv;

#define DETECTION	0
#define TRACKING	1

bool status = 0;
int keyStatus;
Timer periodTimer;

ConnectedComponent shuttlecockL;
ConnectedComponent shuttlecockR;

bool isPointsFound = false;

Mat tmpLeft;
Mat tmpRight;

int main(int argc, char* argv[])
{
	ShuttlecockRecognizer actStereoNo_1("CameraLeft", CAMERA_ARGS_LEFT, "CameraRight", CAMERA_ARGS_RIGHT);
	actStereoNo_1.stereoInit();

	while (!actStereoNo_1.m_bExit)
	{
		actStereoNo_1.syncUpdate();

		imshow("LeftSrc", actStereoNo_1.getFrameLeft());
		imshow("RightSrc", actStereoNo_1.getFrameRight());

		actStereoNo_1.backgroundSubtract();
		actStereoNo_1.preProcessing();

		actStereoNo_1.shuttlecockDetection();

		switch (status)
		{
		case DETECTION:
			break;

		case TRACKING:
			break;

		default:
			break;
		}

		//imshow("Left", actStereoNo_1.getDstLeft());
		//imshow("Right", actStereoNo_1.getDstRight());

		//if push down Esc, kill the progress
		keyStatus = waitKey(1);
		if (keyStatus == 0x1B)
		{
			break;
		}
	}

	return 0;
}